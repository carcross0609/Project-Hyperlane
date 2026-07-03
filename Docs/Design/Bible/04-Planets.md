# Bible 04 — Planets

The places, as content. The planet *data model* (definition vs. state,
population, economy, politics, military) is fixed in SimulationFramework
§2.2 and is not restated here. This section supplies what that model runs
on: the population scale, the **archetype library** that lets ~4,100
catalog systems get believable planet data without hand-authoring, and the
authoring rules for worlds that deserve human attention.

---

## 1. Population Classes

Log scale, `P0–P10`. Population class drives baseline consumption, labor
supply, manpower pools, and how much anyone cares when something happens
there.

| Class | Order of magnitude | Reads as |
|---|---|---|
| P0 | uninhabited | Survey beacon, ruins, nothing |
| P1 | ≤ 10³ | Outpost, listening post, hermits |
| P2 | ≤ 10⁵ | Settlement, mining camp |
| P3 | ≤ 10⁷ | Colony, frontier world |
| P4 | ≤ 10⁸ | Established world, single dominant city |
| P5 | ≤ 10⁹ | Developed world (Naboo, Ryloth) |
| P6 | ≤ 10¹⁰ | Heavy world (Corellia, Kuat) |
| P7 | ≤ 10¹¹ | Regional anchor (Malastare, Duro) |
| P8 | ≤ 10¹² | Hub world (Nal Hutta scale) |
| P9 | ≤ 5·10¹² | Near-total urbanization |
| P10 | ≥ 10¹³ | Ecumenopolis. Coruscant. (There is one.) |

Rule: class moves at most one step per simulated *decade* absent
catastrophe — population is the slowest state in the game, which is what
makes bombardment, plague, and mass-flight events *matter*.

## 2. The Archetype Library

An **archetype** is a planet template: tags, production-slot menu,
population band, government defaults, and texture notes. Archetypes are
data assets; the library below is the tick-zero set (twelve).

| Archetype | Pop band | Produces (slot menu) | Consumes heavily | Texture in one line |
|---|---|---|---|---|
| **Ecumenopolis** | P9–P10 | Government, finance, media, high manufacture | Food (massive), fuel, everything | The city that eats a galaxy's output |
| **Forge World** | P5–P7 | Heavy industry, arms, droids | Ore, fuel, labor | Sky the color of the quota |
| **Shipyard World** | P5–P7 | Hulls (orbital yards), refits | Alloys, components, credits | Cranes visible from orbit; strategic target #1 |
| **Agriworld** | P3–P5 | Food (surplus ×10), textiles | Machinery, fuel | Feeds twenty worlds, owns nothing |
| **Mining World** | P2–P4 | Ore, rare metals, tibanna/gas | Food, equipment, medicine | Company town with a gravity well |
| **Energy World** | P2–P4 | Fuel, tibanna, power cells | Food, machinery | Refineries and flare-light |
| **Trade Nexus** | P5–P8 | Services, finance, *throughput* | Everything, briefly | Warehouses, exchanges, five languages a sentence |
| **Frontier Colony** | P2–P3 | Marginal food, salvage, hope | Tools, medicine, protection | One bad season from empty |
| **Fortress World** | P3–P5 | Garrison capacity, repairs | Supply, materiel | The lane's cork; doctrine with weather |
| **Hutt Fief** | P4–P8 | Vice, salvage, contraband throughput | Luxuries, muscle | Everything is for sale, including the law |
| **Cultural World** | P4–P6 | Art, education, influence, tourism | Luxuries, food | Where the galaxy keeps what it's proud of |
| **Backwater** | P1–P3 | Subsistence | Nearly nothing (off-grid) | The lanes forgot; so did everyone else |

Slot *menus* per archetype are constraints, not guarantees — an individual
world activates a subset (Framework §2.2 production slots), which is where
worlds of the same archetype differ.

## 3. Catalog-Tier Generation (how 4,100 systems get planets)

At Phase 2 import, every catalog system (Bible 01 / Framework §1.2) rolls
lightweight planet records:

1. **Count & habitability** from region-weighted tables (Core rolls rich,
   Wild Space rolls empty; ~35–45% of catalog systems end P0-only).
2. **Archetype** from region-conditioned weights (Outer Rim: mining/
   frontier/backwater-heavy; Core: developed/trade-heavy; Hutt Space:
   fiefs). Grid position near a Major lane biases toward Trade Nexus and
   Fortress; isolation biases Backwater.
3. **Population class** rolled within archetype band, biased by lane
   proximity.
4. **Politics** inherited from the Bible 01 three-tier map; local
   government type from archetype defaults (Framework §2.4).
5. **Deterministic seed:** generation is a pure function of (system ID,
   import version) — regenerating the galaxy is reproducible, and a save
   never stores what the function can re-derive.

Generated planets are *statistical citizens*: they trade, staff cohorts,
pay taxes, and can be fought over — but they have no bespoke content until
promoted to authored (Framework §1.2's promotion path).

## 4. Authored Worlds

Authored planets (the Phase 2 shortlist and everything after) start from
an archetype, then override freely. Authoring rules:

- **Every authored world answers three questions:** what does the war want
  from it, what does the economy want from it, what does a *nobody* do all
  day on it? A world with fewer than three answers stays catalog-tier.
- **One archetype per planet.** Hybrids ("trade nexus but also forge") are
  two planets or a planet-plus-station — mixing muddies both textures and
  the market maths.
- **Stations inherit planet framework** (Framework §2.2): archetype
  `Trade Nexus` or `Fortress` in miniature, parent lane edge or world.
- Landable *locations* per world (Phase 6 content) hang off the
  Content field and never touch sim data.

## 5. Worked Examples (three, spanning the tiers)

- **Kamino** — authored; archetype *Forge World* override "produces:
  clones" (a faction-locked production slot — the entire P2 pressure of
  Bible 02 is one slot on one world). P3, Loyalist enclave (Bible 01
  exception list), Fortress-grade garrison from tick zero.
- **Ryloth** — authored; *Mining World* + P5 population override (a poor
  heavy world — population outsizes its economy; exports ryll, labor, and
  mercenaries; imports everything). Contested, Corellian Run adjacency:
  designed to be fought over early and often.
- **Catalog example** — `System.244Core` (an actual atlas record, Unknown
  Regions): rolls P0-only. It still exists — a place to hide, a place to
  run out of fuel, a name on a chart. Cost to the project: zero.

## 6. What This Section Feeds

- **Bible 06:** archetype production menus are the supply side of the
  commodity roster. **Bible 08:** Fortress/Shipyard/Forge archetypes are
  the strategic target list. **AuthoredSystemsShortlist.md:** every
  shortlist entry cites its archetype + overrides (advances OQ-2).
- **Phase 2 import spec:** §3 is its requirements paragraph.

## Pillar Check

- **1 — Lives:** archetypes give every corner of 4,143 systems an economic
  and political pulse without player-facing intent. *Serves.*
- **5 — Immersion:** archetype texture lines are authoring prompts — every
  world should answer "what does a nobody do here." *Serves.*
- **8 — Expansion:** new archetypes are data; promotion path lets any
  catalog roll become next month's authored world. *Serves.*

## Scalability Check

- Twelve archetypes × region weight tables cover 4,100 systems; authored
  attention stays where authored answers exist. *Passes.*
- Deterministic generation keeps saves small and import re-runnable.
  *Passes.*
- Watch: archetype weights per region need one balancing pass against the
  atlas's real region sizes (1,729 Outer Rim rolls will dominate galactic
  supply totals). Flagged for the Phase 3 economy tuning doc. *Watched.*

---

*Changelog*
- 2026-07-02 — v1. Population classes, 12-archetype library, catalog
  generation rules, authoring rules, worked examples.
