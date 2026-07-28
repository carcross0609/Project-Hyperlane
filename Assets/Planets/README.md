# Planet Database

Reference database of Star Wars planets, scraped from Wookieepedia on
2026-07-03. Sources: the [List of planets](https://starwars.fandom.com/wiki/List_of_planets)
A–Z tables plus the full `Category:Planets` tree (by region / affiliation /
atmosphere / designation / gravity / surface feature). Covers **Canon** and
**Legends**.

This is the most detailed of the asset databases and the hub the others plug
into:

- **Galaxy atlas** — `planet_db.js` (generated here) is loaded by
  `Assets/Map/galaxy_atlas.html`; clicking a system on the map shows that
  planet's data (species, population, climate, terrain, allegiance,
  description) pulled from this database, with a pointer back to the planet's
  folder. Planets matched to a mapped system also carry an `atlas` block in
  their `data.json` with the system name and map coordinates.
- **Species** — each planet's `homeworld_of` lists the species (from
  `Assets/Species`) whose homeworld it is, with relative directory links;
  each species' `data.json` carries `homeworld_planet_directory` pointing back
  here.

## Layout

```
Assets/Planets/
  _Index.json                 # master index: region, sector, system, grid, climate, terrain, population…
  planet_db.js                # generated atlas feed (window.PLANET_DB, keyed by system name)
  <Planet Name>/
    data.json                 # full structured data
    README.md                 # human-readable planetary survey
    Models/                   # planet-specific assets (surface materials, skyboxes, landmarks)
```

Planets in both continuities get `<Name>` (Canon) and `<Name> (Legends)`
directories.

## data.json schema

Same envelope as Starships/Species, plus planet extras:

| Field | Meaning |
|---|---|
| `lead_paragraphs` | The article's **entire lead section**, paragraph by paragraph |
| `article_sections` | Outline of the full Wookieepedia article (indented = subsection) |
| `data` | Cleaned `{{Planet}}` infobox: `region`, `sector`, `system`, `suns`, `moons`, `grid` (Standard Galactic Grid), `diameter`, `atmosphere`, `climate`, `gravity`, `terrain`, `water`, `population`, `species`, `other_species`, `languages`, `government`, `demonym`, `cities`, `imports`, `exports`, `affiliation`, … |
| `homeworld_of` | Species from `Assets/Species` native to this planet (name, directory, continuity) |
| `atlas` | Matching system in `Assets/Map/galaxy_map.json`: name, x/y/z map position, region, grid |

## Regenerating

Pipeline: parse List-of-planets tables + BFS category tree → batch-fetch
wikitext via the MediaWiki API → parse infobox + full lead → cross-link
species index + galaxy map → emit directories → generate `planet_db.js`.
Ask Claude to refresh; keep `planet_db.js` in sync with any regeneration.
