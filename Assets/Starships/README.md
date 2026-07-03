# Starship Database

Reference database of Star Wars starships, scraped from Wookieepedia on
2026-07-03. Covers both **Canon** and **Legends** continuities, in two layers:

1. **Ship types/classes** (~2,900) — from
   [Category:Starship types](https://starwars.fandom.com/wiki/Category:Starship_types),
   recursive. These are the entries that carry class spec sheets and are the
   natural anchors for shared 3D models.
2. **Individual named ships** (~6,300) — from the `Starships by type` and
   `Starships by affiliation` trees under
   [Category:Starships](https://starwars.fandom.com/wiki/Category:Starships)
   (plus Flagships / Personal / Captured / Prototype starships). These have
   `"entry_kind": "individual"` in `data.json` and `"kind": "individual"` in
   the index. In-universe *fictional* ships were deliberately excluded.

## Layout

```
Assets/Starships/
  _Index.json                 # master index: name, type, manufacturer, length, continuity, dir
  <Ship Name>/
    data.json                 # full structured data (see schema below)
    README.md                 # human-readable spec sheet
    Models/                   # drop 3D model assets here (empty for now)
```

Ships that exist in both continuities have two directories: `<Name>` (Canon)
and `<Name> (Legends)`. An individual ship whose name collides with an
existing class directory gets a ` (individual ship)` suffix.

`_Index.json` entries carry `"kind": "type"` or `"kind": "individual"` —
filter on that to load only ship classes (e.g. for the model catalog) or only
named individuals (e.g. for famous-ship encounters).

## data.json schema

| Field | Meaning |
|---|---|
| `name` | Display name of the ship/class |
| `wookieepedia_title` / `wookieepedia_url` | Source page |
| `continuity` | `Canon`, `Legends`, or `Unknown` |
| `infobox_template` | Which Wookieepedia infobox the data came from (`StarshipClass`, `ShipSeries`, `IndividualShip`, …) |
| `description` | First paragraph of the article |
| `data` | All infobox fields, cleaned to plain text — keys vary by ship: `manufacturer`, `line`, `model`, `class`, `cost`, `length`, `width`, `height`, `max_acceleration`, `mglt`, `max_speed`, `hyperdrive`, `shielding`, `hull`, `sensor`, `armament`, `complement`, `crew`, `passengers`, `cargo_capacity`, `consumables`, `role`, `era`, `affiliation`, … |
| `scraped` | Scrape date |

Fields hold either a string or a list of strings (lists preserve the wiki's
bullet structure; two-space indents mark sub-items).

## Notes

- Data quality mirrors Wookieepedia: obscure ships may have only a name and a
  sentence; famous ones have full spec sheets.
- `ShipSeries` pages are product-line overviews (e.g. "X-wing starfighter");
  the hard stats live on the specific model pages (e.g. "T-65B X-wing
  starfighter"), which are also in the database.
- Regenerate/update with the scraper (kept in session scratchpad; re-ask
  Claude to refresh — the pipeline is: enumerate category tree → batch-fetch
  wikitext via the MediaWiki API → parse infoboxes → emit directories).
