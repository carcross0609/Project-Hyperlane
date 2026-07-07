# Free 3D Model Matching

Automated matching of every asset in the reference databases (Starships,
Droids, Weapons, Species, Planets, Characters) against **free, downloadable
3D models on Sketchfab**. Run 2026-07-07.

## What's here

| File | What |
|---|---|
| `sketchfab_sweep.py` | The sweep: one Sketchfab search per asset name (~27.7k queries, adaptive rate limiting, resumable JSONL checkpoint), relevance scoring, then license/facecount enrichment per matched model |
| `write_matches.py` | Distributes sweep results: writes `Models/model_matches.json` into each matched asset dir, inherits class models for unmatched individual starships, regenerates the CSV + gallery |
| `matches.csv` | Flat list of every match (best candidate per asset) |
| `matches.html` | Thumbnail gallery of all direct matches — open locally, filter by category/confidence/name, click through to Sketchfab |

## Where matches land

Each matched asset directory gets `Models/model_matches.json`:
up to 3 candidates with Sketchfab URL, author, CC license, face count,
confidence (`exact` / `strong` / `weak`) and score. Individual starships
with no direct hit inherit their ship class's candidates
(`"source": "inherited-from-class"`).

**Downloading**: Sketchfab free downloads require a (free) account. Every
model carries a Creative Commons license — most require attribution, some
are NC/ND. The project is personal and non-commercial, so NC is fine;
keep author + license from the JSON with any imported model.

## Scoring (why you can mostly trust `exact`/`strong`)

Asset names are tokenized; tokens that don't identify a specific asset
("starfighter", "blaster", "droid", "class"…) get 0.3 weight. Acceptance
requires all distinctive tokens matched (`exact`/`strong`), or half of them
plus explicit Star Wars context in the result (`weak`). Candidates whose
*model name* (not just tags) carries the distinctive tokens rank first, so
"Venator-class Star Destroyer" prefers an actual Venator model over a
popular hallway diorama tagged `venator`. Popularity (likes) breaks ties.
`weak` matches and single-generic-word assets ("Probe droid") deserve a
human eyeball — that's what `matches.html` is for.

## Refreshing

```bash
python3 Assets/ModelMatching/sketchfab_sweep.py \
    --assets Assets --work /tmp/sweep          # hours; resumable, rerun-safe
python3 Assets/ModelMatching/write_matches.py \
    --assets Assets --work /tmp/sweep
```

The sweep is anonymous-API friendly (User-Agent set, ~5 req/s with backoff
on 429). Re-running skips already-checkpointed assets, so a killed run
loses nothing.

## Expectations

The databases deliberately include *every* Wookieepedia entry, most of
which are one-line obscurities no one has ever modeled. Low match rates on
Planets/Species/Individuals are the corpus being exhaustive, not the sweep
failing. The hit rate concentrates exactly where a game needs it: famous
ship classes, iconic droids/weapons, main characters.
