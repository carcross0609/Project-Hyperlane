#!/usr/bin/env python3
"""Sketchfab free-model matching sweep for the Assets/ reference databases.

For every entry in each category _Index.json, queries the Sketchfab search API
(free, downloadable models only), scores results against the asset name, and
records accepted candidates. Fully resumable: results append to a JSONL
checkpoint keyed by "<Category>/<directory>"; rerunning skips done keys.

Phases:
  1. search  — one query per asset name (deduped), score, checkpoint JSONL
  2. license — fetch license/facecount for every accepted model uid (cached)

Writing matches into the asset directories is done by write_matches.py, so the
sweep can be reviewed before touching 27k directories.

Usage:
  python3 sketchfab_sweep.py --assets <repo>/Assets --work <workdir> [--limit N]
"""
import argparse
import json
import os
import re
import sys
import threading
import time
import urllib.error
import urllib.parse
import urllib.request

SEARCH_API = "https://api.sketchfab.com/v3/search"
MODEL_API = "https://api.sketchfab.com/v3/models/"
UA = "ProjectHyperlane-asset-matcher/1.0 (personal non-commercial fan project)"

CATEGORIES = [
    ("Starships", "ships"),
    ("Droids", "droids"),
    ("Weapons", "weapons"),
    ("Species", "species"),
    ("Planets", "planets"),
    ("Characters", "characters"),
]

# Tokens that don't identify a specific asset on their own.
GENERIC = {
    "star", "wars", "starwars", "legends", "canon", "the", "of", "a", "an",
    "and", "or", "in", "ii", "iii", "iv", "unidentified", "class", "type",
    "series", "model", "mark", "mk",
    # ship-ish
    "starfighter", "fighter", "starship", "ship", "cruiser", "destroyer",
    "frigate", "corvette", "transport", "shuttle", "freighter", "gunship",
    "carrier", "dreadnought", "interceptor", "bomber", "yacht", "barge",
    "tug", "hauler", "liner", "skiff", "speeder", "battlecruiser",
    "battleship", "scout", "courier", "tanker",
    # weapon-ish
    "blaster", "pistol", "rifle", "carbine", "cannon", "launcher",
    "detonator", "grenade", "sword", "vibrosword", "vibroblade", "blade",
    "lightsaber", "saber", "staff", "pike", "bow", "slugthrower",
    "disruptor", "turret", "heavy", "light", "repeating", "missile",
    "torpedo", "shell", "bladed", "single", "double",
    # droid-ish
    "droid", "battle", "protocol", "astromech", "probe", "assassin",
    "security", "medical", "labor", "power", "repair", "service", "worker",
    # planet/species-ish
    "planet", "system", "moon", "world", "species",
}

# Parenthetical disambiguators to strip from names before matching.
STRIP_PARENS = re.compile(
    r"\s*\((legends|canon|individual ship|starship|droid|planet|species"
    r"|weapon|lightsaber|character)\)\s*$", re.I)


def squash(s):
    return re.sub(r"[^a-z0-9]+", "", s.lower())


def tokens_of(name):
    name = STRIP_PARENS.sub("", name)
    raw = re.split(r"[^A-Za-z0-9]+", name.lower())
    toks = [t for t in raw if t]
    # collapse hyphenated designations: "t-65b" already split to t, 65b —
    # also add joined runs of consecutive short fragments (t65b)
    joined = squash(name)
    return toks, joined


def clean_name(name):
    return STRIP_PARENS.sub("", name).strip().strip('"')


class RateLimiter:
    """Global adaptive limiter shared by all workers."""

    def __init__(self, interval=0.32):
        self.lock = threading.Lock()
        self.interval = interval
        self.next_ok = 0.0
        self.ok_streak = 0

    def wait(self):
        with self.lock:
            now = time.monotonic()
            t = max(now, self.next_ok)
            self.next_ok = t + self.interval
        delay = t - now
        if delay > 0:
            time.sleep(delay)

    def backoff(self):
        with self.lock:
            self.interval = min(self.interval * 1.6, 3.0)
            self.ok_streak = 0

    def success(self):
        with self.lock:
            self.ok_streak += 1
            if self.ok_streak >= 200 and self.interval > 0.32:
                self.interval = max(0.32, self.interval * 0.8)
                self.ok_streak = 0


def http_json(url, limiter, tries=6):
    err = None
    for attempt in range(tries):
        limiter.wait()
        req = urllib.request.Request(url, headers={"User-Agent": UA})
        try:
            with urllib.request.urlopen(req, timeout=25) as r:
                data = json.load(r)
            limiter.success()
            return data
        except urllib.error.HTTPError as e:
            err = e
            if e.code == 429 or e.code >= 500:
                limiter.backoff()
                time.sleep(min(2 ** attempt, 60))
                continue
            if e.code == 404:
                return None
            raise
        except Exception as e:  # timeouts, transient network
            err = e
            limiter.backoff()
            time.sleep(min(2 ** attempt, 60))
    raise RuntimeError(f"gave up on {url}: {err}")


def trim_result(r):
    thumbs = (r.get("thumbnails") or {}).get("images") or []
    thumb = ""
    if thumbs:
        thumb = min(thumbs, key=lambda t: t.get("width", 1 << 30)).get("url", "")
    user = r.get("user") or {}
    return {
        "uid": r.get("uid"),
        "name": r.get("name") or "",
        "tags": [t.get("name", "") for t in (r.get("tags") or [])],
        "viewerUrl": r.get("viewerUrl", ""),
        "likeCount": r.get("likeCount", 0),
        "viewCount": r.get("viewCount", 0),
        "staffpicked": bool(r.get("staffpickedAt")),
        "author": user.get("displayName") or user.get("username") or "",
        "thumb": thumb,
    }


def score_candidates(asset_name, results):
    toks, _joined = tokens_of(asset_name)
    if not toks:
        return []
    distinctive = [t for t in toks if t not in GENERIC]
    generic_is_all = not distinctive
    if generic_is_all:
        distinctive = toks
    accepted = []
    for r in results:
        text = r["name"] + " " + " ".join(r["tags"])
        stext = squash(text)
        ltext = " " + re.sub(r"[^a-z0-9]+", " ", text.lower()) + " "

        def hit(t):
            if len(t) >= 2:
                return t in stext
            return f" {t} " in ltext

        m_dist = sum(1 for t in distinctive if hit(t))
        m_all = sum(1 for t in toks if hit(t))
        n_dist, n_all = len(distinctive), len(toks)
        # generic tokens count at 30% weight in overall coverage
        w_total = n_dist + 0.3 * (n_all - n_dist) if not generic_is_all else n_all
        w_match = (m_dist + 0.3 * (m_all - m_dist)) if not generic_is_all else m_all
        cov = w_match / w_total if w_total else 0.0
        dcov = m_dist / n_dist if n_dist else 0.0
        ctx = ("starwars" in stext or "swtor" in stext
               or (" star " in ltext and " wars " in ltext))
        if generic_is_all:
            ok = dcov >= 0.99 and ctx
            conf = "strong" if ok else None
        elif dcov >= 0.99 and (ctx or cov >= 0.85):
            conf = "exact" if cov >= 0.99 else "strong"
            ok = True
        elif dcov >= 0.5 and cov >= 0.5 and ctx:
            conf, ok = "weak", True
        else:
            ok, conf = False, None
        if ok:
            # prefer models whose *name* carries the distinctive tokens —
            # tag-only hits are often related scenes (interiors, props),
            # not the asset itself
            sname = squash(r["name"])
            name_hits = sum(1 for t in distinctive if len(t) >= 2 and t in sname)
            name_dcov = name_hits / n_dist if n_dist else 0.0
            accepted.append({**r, "score": round(cov, 3), "confidence": conf,
                             "name_match": round(name_dcov, 2)})
    accepted.sort(key=lambda c: (-c["name_match"], -c["score"], -c["likeCount"]))
    return accepted[:3]


def build_items(assets_dir):
    items = []
    for cat, key in CATEGORIES:
        idx = json.load(open(os.path.join(assets_dir, cat, "_Index.json")))
        for e in idx[key]:
            items.append({
                "category": cat,
                "directory": e["directory"],
                "name": e["name"],
                "kind": e.get("kind", ""),
                "type": e.get("type") or "",
            })
    return items


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--assets", required=True)
    ap.add_argument("--work", required=True)
    ap.add_argument("--limit", type=int, default=0)
    ap.add_argument("--workers", type=int, default=6)
    args = ap.parse_args()
    os.makedirs(args.work, exist_ok=True)
    ckpt_path = os.path.join(args.work, "results.jsonl")

    done = set()
    if os.path.exists(ckpt_path):
        with open(ckpt_path) as f:
            for line in f:
                try:
                    done.add(json.loads(line)["key"])
                except Exception:
                    pass

    items = build_items(args.assets)
    todo = [i for i in items
            if f"{i['category']}/{i['directory']}" not in done]
    if args.limit:
        todo = todo[:args.limit]
    total = len(items)
    print(f"[sweep] {total} assets total, {len(done)} done, {len(todo)} to go",
          flush=True)

    limiter = RateLimiter()
    out_lock = threading.Lock()
    cache_lock = threading.Lock()
    qcache = {}
    counters = {"done": len(done), "matched": 0, "errors": 0}
    out = open(ckpt_path, "a")
    it_lock = threading.Lock()
    it = iter(todo)

    def next_item():
        with it_lock:
            return next(it, None)

    def worker():
        while True:
            item = next_item()
            if item is None:
                return
            key = f"{item['category']}/{item['directory']}"
            q = clean_name(item["name"])
            qnorm = squash(q)
            rec = {"key": key, **item, "query": q}
            try:
                with cache_lock:
                    cached = qcache.get(qnorm)
                if cached is None:
                    url = (SEARCH_API + "?"
                           + urllib.parse.urlencode({
                               "type": "models", "downloadable": "true",
                               "count": "24", "q": f"{q} star wars"}))
                    data = http_json(url, limiter) or {}
                    cached = [trim_result(r) for r in data.get("results", [])]
                    with cache_lock:
                        qcache[qnorm] = cached
                cands = score_candidates(item["name"], cached)
                rec["status"] = "matched" if cands else "none"
                rec["candidates"] = cands
            except Exception as e:
                rec["status"] = "error"
                rec["error"] = str(e)[:300]
                rec["candidates"] = []
            with out_lock:
                out.write(json.dumps(rec, ensure_ascii=False) + "\n")
                out.flush()
                counters["done"] += 1
                if rec["status"] == "matched":
                    counters["matched"] += 1
                elif rec["status"] == "error":
                    counters["errors"] += 1
                if counters["done"] % 250 == 0:
                    print(f"[sweep] {counters['done']}/{total} "
                          f"matched={counters['matched']} "
                          f"errors={counters['errors']} "
                          f"interval={limiter.interval:.2f}s", flush=True)

    threads = [threading.Thread(target=worker, daemon=True)
               for _ in range(args.workers)]
    for t in threads:
        t.start()
    for t in threads:
        t.join()
    out.close()
    print(f"[sweep] search phase complete: {counters}", flush=True)

    # ---- Phase 2: license/geometry enrichment for accepted uids ----
    lic_path = os.path.join(args.work, "model_details.json")
    details = {}
    if os.path.exists(lic_path):
        details = json.load(open(lic_path))
    uids = set()
    with open(ckpt_path) as f:
        for line in f:
            try:
                rec = json.loads(line)
            except Exception:
                continue
            for c in rec.get("candidates", []):
                if c.get("uid"):
                    uids.add(c["uid"])
    missing = sorted(uids - set(details))
    print(f"[license] {len(uids)} unique models, {len(missing)} to fetch",
          flush=True)
    fetched = 0
    for uid in missing:
        try:
            m = http_json(MODEL_API + uid, limiter)
            lic = (m or {}).get("license") or {}
            details[uid] = {
                "license": lic.get("label", ""),
                "license_slug": lic.get("slug", ""),
                "faceCount": (m or {}).get("faceCount"),
                "vertexCount": (m or {}).get("vertexCount"),
                "animationCount": (m or {}).get("animationCount"),
            }
        except Exception as e:
            details[uid] = {"error": str(e)[:200]}
        fetched += 1
        if fetched % 200 == 0:
            json.dump(details, open(lic_path, "w"))
            print(f"[license] {fetched}/{len(missing)}", flush=True)
    json.dump(details, open(lic_path, "w"))
    print("[license] done", flush=True)
    print("[sweep] ALL DONE", flush=True)


if __name__ == "__main__":
    main()
