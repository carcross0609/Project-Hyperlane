# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Project Hyperlane — a single-player, living-galaxy sandbox set in the Star Wars
Republic era, built in **Unreal Engine 5.8 with C++**. Personal, non-commercial
learning project. The galaxy simulates itself (wars, trade, factions) whether or
not the player is watching; the player is one agent among many.

**Current status: Phase 0 (Foundation).** Documentation, repo scaffolding, and
the hand-authored project skeleton exist: `Hyperlane.uproject`, `Source/` with
the initial framework classes (TechnicalArchitecture §7), and `Config/`. The
**first successful compile has not happened yet** — UE 5.8 was still installing
when the skeleton was written. Remaining Phase 0 deliverables: compile + first
editor open, Git LFS verified with a first binary asset, and the proven
edit → compile → hot reload loop.

## The documentation is the source of truth

The design is deliberately documented before it's coded. Read the relevant doc
before changing anything in its area — these define the rules, not just describe
them:

| Doc | Governs |
|---|---|
| `Docs/CorePillars.md` | **Ranked** decision-making rules; lower number wins ties |
| `Docs/TechnicalArchitecture.md` | Layering, modules, the simulation core — the rules of the codebase |
| `Docs/CodingStandards.md` | Naming, Unreal crash-preventers, Definition of Done |
| `Docs/GitWorkflow.md` | Trunk-based flow, LFS, commit format, phase rituals |
| `Docs/Roadmap.md` | Phase goals, deliverables, exit criteria |
| `Docs/Vision.md` | What the game is and (importantly) is not |
| `Docs/Design/SimulationFramework.md` | The simulation GDD — factions, economy, fleets, NPCs; how the galaxy runs itself |
| `Docs/Design/DecisionRegister.md` | Every design decision (DR-IDs) + open questions (OQ-IDs) — check before re-litigating anything |

When a design or technical decision is contested, resolve it by asking "which
pillar wins?" (CorePillars are ranked 1–8; lower wins). Document any knowing
pillar violation in the relevant doc — undocumented violations are treated as bugs.

## Non-negotiable architecture rules

These are the constraints most likely to be violated by code that "just works":

1. **Simulation and presentation are separate worlds.** The galaxy sim is plain
   C++ data and logic and must compile/run with **no** Actor, UI, or rendering
   includes. Actors and UI are *views* onto sim state. This is the single most
   important rule — it makes the "galaxy lives without the player" pillar
   implementable and testable.
2. **Dependencies point downward only.** Four layers, top calls down, never up,
   never sideways-into-internals:
   `PRESENTATION → GAMEPLAY → SIMULATION → CORE`.
   - Simulation → up communicates **only via events** (multicast delegates on
     the owning subsystem). The sim never knows who listened.
   - Gameplay → Simulation goes through **explicit validated command methods**
     (e.g. `TryExecuteTrade(...)` returning a result), never direct mutation.
     The player's UI and an NPC use the *same* command path.
   - Banned: `GetAllActorsOfClass` as discovery; UI holding refs into sim
     internals; anything upward-pointing.
3. **Data-driven for open sets.** Factions, ships, commodities, star systems are
   `UPrimaryDataAsset` subclasses + Gameplay Tags, **never** enums or hardcoded
   lists. Code defines what a faction *can do*; data defines *which* exist.
   DataTables only for genuinely tabular tuning numbers.
4. **`UGalaxySubsystem` (a `UGameInstanceSubsystem`) is the single authoritative
   owner of galaxy state.** It composes plain, unit-testable C++ sub-objects
   (`FGalaxyClock`, `FEconomyModel`, `FFactionModel`, `FTravelNetwork`). The sim
   advances on its **own fixed-timestep clock**, not per rendered frame; sim
   time is a first-class type (`FGalacticDateTime`), never accumulated floats.
5. **Simulation entities are not Actors.** A distant NPC freighter is a struct
   ("traveling Corellia→Duro, 62%"), promoted to a real Actor only when it needs
   to be seen. Sim LOD: `Active` / `Nearby` / `Abstract`.

## Source layout

Single C++ module `Hyperlane` with layer-named folders (a deliberate choice —
physical module split is deferred until it pays for itself; see
TechnicalArchitecture.md §3 for split triggers):

```
Source/Hyperlane/
├── Core/            # shared types, tags, logging, save schema
├── Simulation/      # GalaxySubsystem, clock, economy, factions — NO Actor/UI includes
├── Gameplay/        # Pawns, components, player, interaction
│   ├── Ships/
│   ├── Player/
│   └── Interaction/
└── UI/              # UMG, HUD, galaxy map, sim inspector
```

Folder placement follows the layer map. A file that's hard to place is a design
question — stop and think, don't guess.

## Coding standards essentials

Baseline is the Epic C++ Coding Standard; these are the project's emphases:

- **Epic prefixes always** (`A`ctor, `U`Object, `F`struct, `E`num, `I`nterface,
  `T`emplate, `b`Bool). PascalCase for everything (Unreal style, not camelCase).
  No project prefix on class names.
- **Every `UObject*` member is `UPROPERTY() TObjectPtr<T>`** — no exceptions; raw
  UObject pointers are invisible to the GC (the #1 new-dev crash). Non-owning:
  `TWeakObjectPtr<T>`, checked before use.
- **Tick is opt-in.** Constructors set `PrimaryActorTick.bCanEverTick = false`
  unless the class earns it. Sim work runs on the sim clock, not Actor ticks.
- `check()` for impossible-by-construction, `ensure()` for shouldn't-happen-but-
  recoverable, plain `if` for expected runtime conditions. Never validate data/
  user input with `check`.
- Forward-declare in headers, include in .cpp. One public class per header;
  filename == class name without prefix (`ShipPawn.h` for `AShipPawn`).
- Every file starts with the copyright line from CodingStandards.md.
- `TODO(phase-N): reason` — TODOs carry a phase and reason or get deleted.
- **C++ is the source of truth for all logic**; Blueprint only for data assets,
  binding meshes/VFX to C++ subclasses, and UMG layout — never system/sim logic.

**Definition of Done** (every change): compiles with zero new warnings; obeys the
layer rules (self-review the diff for upward deps); tunables live in data not
literals; sim systems ship a log category *and* an inspector view in the same
change; diff self-reviewed like a hostile senior engineer would.

## Building and running

On macOS:

- Requires **Xcode** (launched once to accept licenses), **Unreal Engine 5.8**
  via the Epic Games Launcher, and **Git LFS** (`brew install git-lfs && git lfs install`).
- Open `Hyperlane.uproject`; the editor offers to compile C++ on first open.
- Regenerate IDE project files via right-click `.uproject` → Generate Project
  Files (the generated `*.xcodeproj`/`*.sln` are git-ignored).
- No automated test framework exists yet. Sim-only automated testing (running the
  Simulation layer headless in a commandlet) is a planned Phase 3 capability —
  it's a reason the sim layer must never include Actor/UI code.

## Git workflow

- **Trunk-based.** `main` **always compiles and always opens in the editor** —
  this invariant is the whole model. Never commit a broken build to `main`.
- Short-lived branches (`feature/…`, `fix/…`, `docs/…`); merge features with
  `--no-ff`. Tiny changes may go straight to `main`. Rebase local work freely
  before it's pushed.
- **Git LFS is required and non-negotiable** — all binary content (`.uasset`,
  `.umap`) and art/audio sources route through LFS via `.gitattributes`. Verify
  `git lfs install` before the first content commit. Binary assets cannot be
  merged: never edit the same asset on two live branches.
- Commit messages: imperative subject ≤72 chars; body explains *why* and records
  known limitations.
- **Phase boundary ritual:** sweep `TODO(phase-N)` comments, write
  `Docs/Retrospectives/Phase-N.md`, then tag (`phase-0`, `phase-1`, …) and push
  to the private remote.

## Deferred decisions (don't adopt these without revisiting the doc)

Explicitly *not* being used yet, to avoid adopting them by accident:
Gameplay Ability System (D1, revisit Phase 5), physical module split (D2),
gameplay message bus (D3, plain multicast delegates for now), Mass Entity (D4),
World Partition (D5), multiplayer (D6 — never, net-aware discipline only). See
TechnicalArchitecture.md §10.

## Environment note

This repo lives under `~/Desktop`, which is synced by iCloud Drive. Files may be
"dataless" placeholders that block on read until iCloud materializes them; if a
read times out with `Operation timed out`, the file needs downloading (Finder →
right-click → Download Now, or `brctl download <path>`) before it can be read.
