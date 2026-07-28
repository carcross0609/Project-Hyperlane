# Roadmap

A multi-year plan, structured so that **something is always playable** and
**every phase teaches specific skills**. Dates are deliberately absent —
phases end when their exit criteria pass, not when a calendar says so.

Each phase has:
- **Goal** — the one sentence that defines done.
- **Deliverables** — concrete, demoable artifacts.
- **Learning track** — the C++/Unreal concepts this phase exists to teach.
- **Exit criteria** — objective checks before moving on.

The golden rule: **do not start phase N+1 to escape the boring end of
phase N.** Finishing is the skill being practiced.

---

## Phase 0 — Foundation (current)

**Goal:** a project a professional studio would recognize: documented vision,
architecture, standards, and an empty-but-compiling Unreal project under
version control.

**Deliverables**
- [x] Vision.md, CorePillars.md, Roadmap.md
- [x] TechnicalArchitecture.md, CodingStandards.md, GitWorkflow.md
- [x] Repo scaffolding (.gitignore, .gitattributes, .editorconfig, README)
- [x] UE 5.8 C++ project created (`Hyperlane.uproject`), compiles, runs
- [x] Git LFS verified working with a first binary asset
- [x] Dev environment proven: edit C++ → compile → hot reload → see change

**Learning track:** what a UE project *is* — modules, targets, Build.cs, the
editor/game relationship, how C++ becomes a running game.

**Exit criteria:** clean clone on a fresh folder builds and runs; you can
explain every generated file at the project root.

---

## Phase 1 — One Ship, One System

**Goal:** fly a ship, with feel, inside a single handcrafted star system.

**Deliverables**
- [ ] Ship pawn with 6-DOF-ish flight (arcade-leaning, tunable from data)
- [ ] Enhanced Input setup (keyboard/mouse + gamepad from day one)
- [ ] One star system level: a planet, a station, ambient traffic *placeholder*
- [ ] Docking interaction (approach station → docked state → undock)
- [ ] Debug HUD (velocity, throttle) — ugly is fine, diegetic comes later

**Learning track:** Actor/Pawn/Component model, UPROPERTY & garbage
collection, input handling, tick vs physics, the component-first pattern
we'll use everywhere.

**Exit criteria:** flying feels *good* to you for 10+ minutes; all flight
tuning lives in a data asset, not in code constants.

---

## Phase 2 — A Galaxy of Data

**Goal:** the galaxy exists as data — many systems, connected by hyperlanes,
travelable.

**Deliverables**
- [ ] Star system, planet, and hyperlane definitions as data assets
- [ ] 10–15 authored systems (Coruscant, Corellia, Tatooine, Ryloth…)
- [ ] Galaxy map screen (functional, not pretty)
- [ ] Hyperspace travel: real transition with real travel time, not teleport
- [ ] `GalaxySubsystem` established as the single owner of galaxy state

**Learning track:** data-driven design (PrimaryDataAssets, Asset Manager,
Gameplay Tags), subsystems, level streaming/travel, save/load fundamentals.

**Exit criteria:** adding a new star system requires **zero code** — only a
data asset and content; game state survives save/quit/load mid-journey.

---

## Phase 3 — A Living Economy (v0)

**Goal:** the first system that satisfies Pillar 1 — prices move because
simulated supply and demand move, not because the player traded.

**Deliverables**
- [ ] Commodities as data; stations produce/consume on the sim clock
- [ ] Simulation tick architecture (fixed-timestep, runs while playing)
- [ ] Market UI at stations: buy/sell, cargo hold, credits
- [ ] Abstract NPC traders moving goods between systems (no ships rendered —
      pure simulation entities first)
- [ ] Sim inspector debug screen: watch the economy from above
- [ ] The core loop closes: **buy low → fly → sell high → upgrade**

**Learning track:** simulation architecture, sim/presentation separation,
UMG/UI in C++, serialization of evolving state, debugging tools as a habit.

**Exit criteria:** run the sim 1 hour untouched → economy state has changed
plausibly and you can explain why from the inspector; a price exploit you
design on purpose gets arbitraged away by NPC traders.

---

## Phase 4 — The Galaxy Moves

**Goal:** factions with goals act on the galaxy; the player can *see* it.

**Deliverables**
- [ ] Factions as data (Republic, Separatists, Hutts, independents)
- [ ] Faction AI v0: territory, resources, simple strategic goals (blockade,
      expand, raid) — outcomes abstract-resolved, not battle-simulated
- [ ] Sim events surface diegetically: HoloNet news feed, market rumors
- [ ] Sim consequences touch the player: blockades close hyperlanes, war
      zones change prices and traffic
- [ ] Reputation v0: factions remember the player

**Learning track:** utility-based AI decision-making, event-driven
architecture at scale, simulation level-of-detail (near/far fidelity).

**Exit criteria:** two untouched 2-hour runs produce visibly different
galaxies; a war changed at least one of your trade routes and you found out
*in-fiction* before you found out from a debug screen.

---

## Phase 5 — Danger (Space Combat)

**Goal:** space combat that the simulation uses — pirates, patrols,
escort work, bounty hunting v0.

**Deliverables**
- [ ] Weapons/damage/destruction for ships (data-driven, reusable for ground
      combat later)
- [ ] NPC combat pilots (behavioral AI, distinct from strategic faction AI)
- [ ] Ambient encounters generated *by sim state* (pirates where trade is
      rich and patrols are thin — never random spawns)
- [ ] First career loops beyond trading: bounty board, escort contracts

**Learning track:** combat architecture, AI behaviors, and the **GAS
decision**: we evaluate the Gameplay Ability System here, when we can judge
it, not in Phase 0 when it would judge us.

**Exit criteria:** a pirate that attacks you exists because the sim decided
piracy pays there; kill him and that number moves.

---

## Phase 6 and Beyond — Boots on the Ground (sketch only)

Deliberately under-planned; re-plan after Phase 5 retrospective.

- Character-scale gameplay: stations/cities interiors, third-person movement
- Ground combat, blasters → eventually lightsabers and Force careers
- Deeper careers: politics, military enlistment, fleet command
- NPC life simulation at character scale
- The Jedi/Sith path — last, because it must sit *on top of* every system
  above (reputation, factions, combat, economy) to feel earned

---

## Standing Rules

1. **Re-plan at every phase boundary.** This roadmap is a hypothesis; the
   retrospective is where it meets reality.
2. **Each phase ends with a tagged release** (`phase-1`, `phase-2`…) and a
   short written retrospective in `Docs/Retrospectives/`.
3. **Scope cuts come from the bottom of the phase's deliverable list**, never
   from testing/polish of what's built.
