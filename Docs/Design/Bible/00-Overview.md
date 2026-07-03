# Game Design Bible — Overview

The Bible is the project's **design truth**: which factions exist, what the
political map looks like at start, what a CR90 corvette is, who lives on
Ryloth. It is the content companion to two documents it must never duplicate:

- **`Docs/CorePillars.md`** — the ranked decision rules. The Bible cites
  pillars; it does not restate them.
- **`Docs/Design/SimulationFramework.md`** — the mechanics. The Framework
  says *factions have doctrine weights*; the Bible says *what the Trade
  Federation's weights are, and why*. When a Bible section describes
  behavior, it references the Framework section that implements it.
- **`Docs/Design/DecisionRegister.md`** — decisions already made, with
  rationale (DR-IDs) and open questions (OQ-IDs). Bible sections cite IDs
  instead of re-arguing settled questions.

Everything in the Bible is **seed data by design** (Pillar 2): it describes
the galaxy at tick zero and the pressures acting on it — never what happens
next. If a Bible statement can only stay true via scripting, it's wrong.

## Era Statement

**The simulation begins in 22 BBY, days after the Battle of Geonosis.** The
Clone Wars have just ignited: the Confederacy's secession is formalized, the
Republic's clone army is real and mobilizing, and every hyperlane in the
galaxy is about to matter. From tick one, history belongs to the simulation
— canon defines the starting map, the actors, and the standing pressures,
and nothing after that. The war can be won, lost, stalemated, or outlived.

## Continuity Policy

**Legends-primary blend.** The Legends continuity supplies the spine — the
galaxy atlas (4,143 systems, the named trade routes, the sector map), the
political geography, the economic lore. Material from newer canon is
cherry-picked freely where it's better raw material for a systems game.
Consistency with the simulation outranks consistency with any published
timeline. (Non-commercial project; purity buys nothing.)

## Sections

| # | File | Covers |
|---|---|---|
| 01 | `01-GalaxyAndHyperlanes.md` | Regions, political map at start, hyperlane structure |
| 02 | `02-TimelineAndEraRules.md` | The war's opening state, standing pressures, era constraints |
| 03 | `03-Factions.md` | Major and minor factions: identity, doctrine, holdings |
| 04 | `04-Planets.md` | Planet data framework and the archetype library |
| 05 | `05-Ships.md` | Ship class taxonomy and attribute model |
| 06 | `06-EconomyAndTrade.md` | Commodities, production chains, trade content |
| 07 | `07-NPCLife.md` | Population, roles, and persons — the people content |
| 08 | `08-FleetsAndWarfare.md` | Military organization, doctrine, war content |
| 09 | `09-PlayerPaths.md` | Careers and progression content |

Sections are written in order; later sections may sharpen earlier ones (with
changelog entries), never silently contradict them. Every section ends with
a **Pillar Check** and a **Scalability Check** — the second asks "does this
content survive contact with 4,143 systems and a multi-year project?"

---

*Changelog*
- 2026-07-02 — v1. Bible established; era, continuity, and structure fixed.
