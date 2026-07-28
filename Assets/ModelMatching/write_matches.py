#!/usr/bin/env python3
"""Distribute Sketchfab sweep results into the asset directories.

Reads the sweep checkpoint (results.jsonl + model_details.json) produced by
sketchfab_sweep.py and:
  1. standardizes every planet's empty `Assets/` subdir to `Models/`
  2. writes `Models/model_matches.json` into each matched asset directory
  3. gives unmatched *individual* starships an inherited match from their
     ship class (index `type` field -> class name), marked "inherited"
  4. regenerates ModelMatching/matches.csv, matches.html (thumbnail gallery)
     and the stats block printed at the end

Usage:
  python3 write_matches.py --assets <repo>/Assets --work <workdir> [--dry-run]
"""
import argparse
import csv
import html
import json
import os
import re
from collections import defaultdict

STRIP_PARENS = re.compile(
    r"\s*\((legends|canon|individual ship|starship|droid|planet|species"
    r"|weapon|lightsaber|character)\)\s*$", re.I)


def squash(s):
    return re.sub(r"[^a-z0-9]+", "", s.lower())


def clean(s):
    return STRIP_PARENS.sub("", s or "").strip()


def load_results(work):
    recs = {}
    with open(os.path.join(work, "results.jsonl")) as f:
        for line in f:
            try:
                r = json.loads(line)
                recs[r["key"]] = r
            except Exception:
                pass
    details_path = os.path.join(work, "model_details.json")
    details = {}
    if os.path.exists(details_path):
        details = json.load(open(details_path))
    return recs, details


def enrich(cand, details):
    d = details.get(cand.get("uid", ""), {})
    return {
        "name": cand["name"],
        "url": cand["viewerUrl"],
        "uid": cand["uid"],
        "author": cand["author"],
        "license": d.get("license", ""),
        "faceCount": d.get("faceCount"),
        "animationCount": d.get("animationCount"),
        "confidence": cand["confidence"],
        "score": cand["score"],
        "likes": cand["likeCount"],
        "thumbnail": cand["thumb"],
    }


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--assets", required=True)
    ap.add_argument("--work", required=True)
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--date", default="2026-07-07")
    args = ap.parse_args()
    recs, details = load_results(args.work)

    # 1. planets: Assets/ -> Models/
    renamed = 0
    planets_root = os.path.join(args.assets, "Planets")
    for d in os.listdir(planets_root):
        p = os.path.join(planets_root, d)
        old, new = os.path.join(p, "Assets"), os.path.join(p, "Models")
        if os.path.isdir(old) and not os.path.isdir(new):
            if not args.dry_run:
                os.rename(old, new)
            renamed += 1
    print(f"planet Assets/ -> Models/ renames: {renamed}")

    # 3-prep. class model map for individual-starship inheritance
    ships_idx = json.load(
        open(os.path.join(args.assets, "Starships", "_Index.json")))["ships"]
    class_map = {}  # squashed class name -> (directory, record)
    for e in ships_idx:
        if e.get("kind") != "type":
            continue
        rec = recs.get(f"Starships/{e['directory']}")
        if rec and rec.get("candidates"):
            class_map.setdefault(squash(clean(e["name"])), (e["directory"], rec))

    stats = defaultdict(lambda: defaultdict(int))
    rows = []
    gallery = []
    written = inherited = 0

    for key, rec in sorted(recs.items()):
        cat, directory = rec["category"], rec["directory"]
        stats[cat]["total"] += 1
        stats[cat][rec["status"]] += 1
        cands = rec.get("candidates") or []
        source = "sketchfab-search"
        inherited_from = None

        if not cands and cat == "Starships" and rec.get("kind") == "individual":
            # `type` comes from the infobox: usually a string, sometimes a
            # list of classifications — try each until a class matches
            t = rec.get("type") or ""
            hit = None
            for tn in (t if isinstance(t, list) else [t]):
                hit = class_map.get(squash(clean(str(tn))))
                if hit:
                    break
            if hit:
                class_dir, class_rec = hit
                cands = class_rec["candidates"]
                source = "inherited-from-class"
                inherited_from = class_dir
                stats[cat]["inherited"] += 1
                inherited += 1

        if not cands:
            continue

        asset_dir = os.path.join(args.assets, cat, directory)
        if not os.path.isdir(asset_dir):
            print(f"  !! missing dir {cat}/{directory}")
            continue
        payload = {
            "asset": rec["name"],
            "matched": args.date,
            "source": source,
            "note": ("Free downloadable models found on Sketchfab (login "
                     "required to download; respect each model's CC license "
                     "terms, esp. attribution)."),
            "candidates": [enrich(c, details) for c in cands],
        }
        if inherited_from:
            payload["inherited_from_class"] = inherited_from
        if not args.dry_run:
            os.makedirs(os.path.join(asset_dir, "Models"), exist_ok=True)
            with open(os.path.join(asset_dir, "Models",
                                   "model_matches.json"), "w") as f:
                json.dump(payload, f, indent=2, ensure_ascii=False)
        written += 1
        best = payload["candidates"][0]
        rows.append([cat, rec.get("kind", ""), directory, rec["name"],
                     best["name"], best["author"], best["license"],
                     best["confidence"] if source == "sketchfab-search"
                     else "inherited", best["score"], best["faceCount"] or "",
                     best["url"]])
        if source == "sketchfab-search":
            gallery.append({
                "cat": cat, "asset": rec["name"], "dir": directory,
                "kind": rec.get("kind", ""), "m": best["name"],
                "a": best["author"], "lic": best["license"],
                "conf": best["confidence"], "score": best["score"],
                "likes": best["likes"], "url": best["url"],
                "thumb": best["thumbnail"],
                "alts": [{"m": c["name"], "url": c["url"],
                          "conf": c["confidence"]}
                         for c in payload["candidates"][1:]],
            })

    out_dir = os.path.join(args.assets, "ModelMatching")
    if not args.dry_run:
        with open(os.path.join(out_dir, "matches.csv"), "w", newline="") as f:
            w = csv.writer(f)
            w.writerow(["category", "kind", "directory", "asset",
                        "model", "author", "license", "confidence",
                        "score", "faces", "url"])
            w.writerows(rows)
        write_gallery(os.path.join(out_dir, "matches.html"), gallery)

    print(f"\nper-asset match files written: {written} "
          f"(of which inherited: {inherited})")
    for cat in sorted(stats):
        s = stats[cat]
        direct = s.get("matched", 0)
        print(f"  {cat:12s} total={s['total']:6d} direct={direct:5d} "
              f"inherited={s.get('inherited', 0):5d} "
              f"none={s.get('none', 0):6d} errors={s.get('error', 0)}")


def write_gallery(path, gallery):
    data = json.dumps(gallery, ensure_ascii=False).replace("</", "<\\/")
    page = """<!DOCTYPE html>
<html><head><meta charset="utf-8">
<title>Hyperlane — Free 3D Model Matches</title>
<style>
 body{font-family:-apple-system,Helvetica,sans-serif;background:#101318;color:#dde3ee;margin:0}
 header{padding:14px 20px;background:#181d26;position:sticky;top:0;z-index:2;
        display:flex;gap:12px;align-items:center;flex-wrap:wrap}
 h1{font-size:16px;margin:0 12px 0 0}
 input,select{background:#0d1015;color:#dde3ee;border:1px solid #333c4d;
        border-radius:6px;padding:6px 10px;font-size:13px}
 #count{color:#8fa0bd;font-size:12px}
 #grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(230px,1fr));
       gap:12px;padding:16px 20px}
 .card{background:#181d26;border-radius:10px;overflow:hidden;font-size:12px}
 .card img{width:100%;height:130px;object-fit:cover;display:block;background:#0d1015}
 .card .p{padding:8px 10px}
 .asset{font-weight:600;font-size:13px;margin-bottom:2px}
 .m{color:#9db4d8}
 .meta{color:#7d8ba3;margin-top:4px;display:flex;gap:6px;flex-wrap:wrap}
 .b{padding:1px 6px;border-radius:8px;background:#232b3a;font-size:10px}
 .exact{background:#1d4429;color:#7fe0a0}.strong{background:#3d3a1d;color:#e0d47f}
 .weak{background:#442222;color:#e09f7f}
 a{color:inherit;text-decoration:none}
 .alts{margin-top:3px;font-size:10px;color:#6d7a91}
 .alts a{color:#8fa0bd;text-decoration:underline}
</style></head><body>
<header><h1>Free 3D Model Matches</h1>
<select id="cat"><option value="">All categories</option></select>
<select id="conf"><option value="">All confidence</option>
<option>exact</option><option>strong</option><option>weak</option></select>
<input id="q" placeholder="filter by name…" size="28">
<span id="count"></span></header>
<div id="grid"></div>
<script>
const DATA = __DATA__;
const cats = [...new Set(DATA.map(d=>d.cat))].sort();
const catSel = document.getElementById('cat');
cats.forEach(c=>{const o=document.createElement('option');o.textContent=c;catSel.appendChild(o)});
const grid = document.getElementById('grid'), count = document.getElementById('count');
function render(){
  const c=catSel.value, f=document.getElementById('conf').value,
        q=document.getElementById('q').value.toLowerCase();
  const rows=DATA.filter(d=>(!c||d.cat===c)&&(!f||d.conf===f)&&
    (!q||d.asset.toLowerCase().includes(q)||d.m.toLowerCase().includes(q)));
  count.textContent=rows.length+' / '+DATA.length+' matches';
  grid.innerHTML=rows.slice(0,800).map(d=>`
   <div class="card"><a href="${d.url}" target="_blank">
    <img loading="lazy" src="${d.thumb}" alt=""></a><div class="p">
    <div class="asset">${esc(d.asset)}</div>
    <div class="m"><a href="${d.url}" target="_blank">${esc(d.m)}</a></div>
    <div class="meta"><span class="b ${d.conf}">${d.conf} ${d.score}</span>
     <span class="b">${esc(d.cat)}</span>
     <span class="b">${esc(d.lic||'license?')}</span>
     <span class="b">♥ ${d.likes}</span></div>
    <div class="meta">by ${esc(d.a)}</div>
    ${d.alts.length?`<div class="alts">alts: ${d.alts.map(a=>
      `<a href="${a.url}" target="_blank">${esc(a.m.slice(0,30))}</a>`).join(' · ')}</div>`:''}
   </div></div>`).join('');
}
function esc(s){return (s||'').replace(/[&<>"]/g,m=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[m]))}
['cat','conf','q'].forEach(id=>document.getElementById(id)
  .addEventListener(id==='q'?'input':'change',render));
render();
</script></body></html>"""
    with open(path, "w") as f:
        f.write(page.replace("__DATA__", data))


if __name__ == "__main__":
    main()
