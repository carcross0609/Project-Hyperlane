# Species Database

Reference database of Star Wars sentient species, scraped from
[Wookieepedia's Sentient species category](https://starwars.fandom.com/wiki/Category:Sentient_species)
(recursively — by ability/biology/location/society, extinct, and hybrid
subtrees) on 2026-07-03. Covers both **Canon** and **Legends** continuities.
In-universe *fictional* species were deliberately excluded, matching the
starship database's rule.

## Layout

```
Assets/Species/
  _Index.json                 # master index: name, designation, classification, homeworld, language, dir
  <Species Name>/
    data.json                 # full structured data
    README.md                 # human-readable summary
    Models/                   # character/creature model assets (empty for now)
```

Species existing in both continuities have two directories: `<Name>` (Canon)
and `<Name> (Legends)`.

## data.json schema

Same envelope as `Assets/Starships` (`name`, `wookieepedia_title`,
`wookieepedia_url`, `continuity`, `infobox_template`, `description`, `data`,
`scraped`). The `data` object holds the cleaned `{{Species}}` infobox fields —
keys vary by article: `designation` (sentient/semi-sentient), `class`
(classification, e.g. Near-human, Mammal), `subspecies`, `races`, `height`,
`mass`, `skincolor`, `haircolor`, `eyecolor`, `distinctions`, `lifespan`,
`origin` (homeworld), `habitat`, `diet`, `language`, …

Fields hold either a string or a list of strings; two-space indents in lists
mark sub-items.

## Notes

- Data quality mirrors Wookieepedia: major species have full biology infoboxes,
  one-off background species may be a name and a sentence.
- Unlike ships there is no type/individual split — every entry is a species.
- Regeneration pipeline: enumerate category tree → batch-fetch wikitext via the
  MediaWiki API → parse infoboxes → emit directories (ask Claude to refresh).
