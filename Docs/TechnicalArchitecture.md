# Technical Architecture

The rules of the codebase. Feature design docs live elsewhere; this document
defines the structure they must fit into. It evolves — by edit, with a note in
the changelog at the bottom — but it is never ignored silently.

---

## 1. Guiding Technical Principles

1. **Simulation and presentation are separate worlds.** The galaxy sim is
   plain data and logic. It never touches Actors, UI, or rendering. Actors and
   UI are *views* onto sim state. This is the single most important rule in
   the codebase — it's what makes Pillar 1 (the galaxy lives) implementable
   and testable.
2. **Data-driven by default.** Open sets (factions, ships, commodities, star
   systems) are assets, never enums or hardcoded lists. Code defines *what a
   faction can do*; data defines *which factions exist*.
3. **Event-driven communication.** Systems announce ("hyperlane blockaded"),
   they don't command other systems. Listeners subscribe. No system holds a
   hard pointer into another system's internals.
4. **Component-first gameplay.** Behavior lives in components composed onto
   thin Actors. A ship is a Pawn with movement, docking, and cargo components
   — later, an NPC freighter reuses those exact components.
5. **Single-player, net-aware.** No replication code, but its disciplines:
   game logic never lives in UI; state changes flow through authoritative
   systems, not from whoever's Tick got there first; nothing assumes "the
   player" is the only agent that acts on the world (Pillar 1 demands this
   anyway — NPCs use the same verbs).
6. **Debuggability is a feature.** Every simulation system ships with an
   inspector view and log category in the same PR. On a systems game, you
   cannot fix what you cannot see.

## 2. Layered Architecture

Four layers. Dependencies point **downward only** — a layer may include and
call the layers below it, never above, never sideways-into-internals.

```
┌────────────────────────────────────────────────────┐
│  PRESENTATION   UI (UMG), HUD, galaxy map, audio,  │
│                 VFX, HoloNet news display          │
├────────────────────────────────────────────────────┤
│  GAMEPLAY       Pawns, components, player          │
│                 controller, input, interaction,    │
│                 rendered NPCs, cameras             │
├────────────────────────────────────────────────────┤
│  SIMULATION     GalaxySubsystem, sim clock,        │
│                 economy, factions, abstract NPCs,  │
│                 hyperlanes — NO Actor/UI includes  │
├────────────────────────────────────────────────────┤
│  CORE           Shared types, gameplay tags,       │
│                 utilities, logging, save schema    │
└────────────────────────────────────────────────────┘
```

Consequences worth spelling out:

- The Simulation layer compiles without any gameplay class existing. It could
  in principle run in a commandlet with no world loaded (and eventually will,
  for automated sim testing — see Roadmap Phase 3 exit criteria).
- Upward communication happens only via **events**: Simulation broadcasts
  "battle resolved at Ryloth"; Gameplay/Presentation subscribe and spawn
  wreckage or a news item. Simulation never knows who listened.
- Gameplay *reads* sim state through query interfaces and *writes* through
  explicit commands ("sell 20 tibanna at this station"), which the sim
  validates. The player's market UI and an NPC trader use the same command.

## 3. Module Strategy (and why we start with one)

Unreal supports splitting a game into C++ modules with enforced dependency
boundaries. The target end-state looks like:

| Module (future)        | Contents                          | May depend on |
|------------------------|-----------------------------------|---------------|
| `HyperlaneCore`        | Core layer                        | engine only   |
| `HyperlaneSim`         | Simulation layer                  | Core          |
| `Hyperlane` (primary)  | Gameplay layer, game framework    | Sim, Core     |
| `HyperlaneUI`          | Presentation layer                | all below     |

**But we start with a single module (`Hyperlane`) using this folder layout:**

```
Source/Hyperlane/
├── Core/            # future HyperlaneCore
├── Simulation/      # future HyperlaneSim
├── Gameplay/
│   ├── Ships/
│   ├── Player/
│   └── Interaction/
└── UI/              # future HyperlaneUI
```

Why not split immediately, given Pillar 4 (modularity)? Because for a
developer new to Unreal, multi-module setups multiply confusing failure modes
(export macros, circular link errors, Build.cs dependency debugging) before
you've written a line of gameplay. Modularity is first achieved by
**discipline** (the layer rules above, enforced in self-review), then made
**physical** when it pays for itself.

**Split triggers** — we create the real modules when any of these fires:
- The layer rules keep getting violated accidentally → make the compiler
  enforce them (this is the most likely trigger, expected around Phase 3 when
  the Simulation layer becomes substantial).
- Editor/game compile times grow past ~2 minutes for small changes.
- We want sim-only automated tests running without gameplay code.

The folder layout above makes the future split mostly mechanical.

## 4. The Simulation Core

The beating heart of Pillar 1. Key decisions:

### 4.1 GalaxySubsystem owns everything

`UGalaxySubsystem` (a `UGameInstanceSubsystem`) is the single authoritative
owner of galaxy state. *Why a GameInstance subsystem:* it is created once at
game start and survives level loads — fly from Coruscant to Tatooine, the
galaxy sim never blinks. Subsystems also give us singleton access without
writing singleton code (lifetime, discovery, and shutdown handled by the
engine).

It composes focused sub-objects rather than doing work itself:
`FGalaxyClock`, `FEconomyModel`, `FFactionModel`, `FTravelNetwork` — each a
plain C++ class, unit-testable, unaware of the subsystem hosting it.

### 4.2 The sim runs on its own clock

Simulation advances in **fixed timesteps** (initial choice: one sim tick per
real second, with galaxy-scale processes bucketed across ticks so no single
frame pays for the whole galaxy). It does *not* do work per rendered frame.

- Fixed steps make the sim deterministic-ish, debuggable, and testable
  ("advance 1 week, assert on state").
- Time scale is a sim parameter, enabling fast-forward for both debugging and
  gameplay (hyperspace journeys can compress time).
- Sim time is a first-class type (`FGalacticDateTime`), never `float Seconds`
  accumulated ad hoc.

### 4.3 Simulation entities are not Actors

An NPC trade freighter three sectors away is a struct in an array — position
abstracted to "traveling Corellia→Duro, 62% complete". Only when something
needs to be *seen* does the Gameplay layer spawn an Actor to represent it,
and that Actor is a disposable puppet: the sim entity remains the truth.

This is simulation **level-of-detail**, and it's the only way a galaxy of
thousands of agents runs on one machine:

| Sim LOD    | Who                                  | Fidelity                          |
|------------|--------------------------------------|-----------------------------------|
| `Active`   | Player's immediate bubble            | Real Actors, physics, full AI     |
| `Nearby`   | Player's star system                 | Lightweight entities, coarse tick |
| `Abstract` | Everywhere else                      | Statistical/aggregate simulation  |

Entities promote/demote between LODs as the player moves. We build `Abstract`
first (Phase 3) — it's the layer that makes the galaxy alive.

## 5. Data-Driven Design

- **`UPrimaryDataAsset` subclasses** define the galaxy's nouns:
  `UStarSystemDef`, `UPlanetDef`, `UFactionDef`, `UShipClassDef`,
  `UCommodityDef`. Designers (i.e., you, in a different mood) create these in
  the editor without touching code. Loaded via the **Asset Manager** so the
  whole galaxy's data isn't resident at once.
- **Gameplay Tags** (`Faction.Republic`, `Ship.Role.Freighter`,
  `Commodity.Contraband`) for taxonomy and matching — hierarchical, data-editable,
  and far more flexible than enums for open sets (Pillar 8).
- **DataTables** only for genuinely tabular tuning data (price curves, spawn
  weights). Assets for identity, tables for numbers.
- Every runtime-persistent struct carries a **schema version** from day one.
  Save compatibility is a Phase 2 exit criterion, not a retrofit.

## 6. Communication Patterns

- **Within a layer:** direct calls are fine. Don't event-bus your own leg.
- **Simulation → up:** multicast delegates on the owning subsystem to start
  (e.g. `OnHyperlaneStatusChanged`). If subscriber sprawl becomes painful
  (~Phase 4), we adopt a gameplay message bus keyed by Gameplay Tags. Logged
  as deferred decision D3.
- **Gameplay → Simulation:** explicit command methods with validation
  (`TryExecuteTrade(...)` returning a result enum) — never direct mutation of
  sim state. The market UI and NPC traders go through the same door.
- **Never:** `GetAllActorsOfClass` as a discovery mechanism; UI holding
  references into sim internals; anything upward-pointing.

## 7. Initial Class Hierarchy (Phases 0–2)

Deliberately small. Classes earn their existence.

```
Game framework (thin glue)
├── UHyperlaneGameInstance          # owns lifetime; little logic of its own
├── AHyperlaneGameMode              # per-map rules; nearly empty for now
├── AHyperlanePlayerController      # input routing, possession
└── UHyperlaneAssetManager          # asset loading policy

Core
├── FGalacticDateTime               # sim time as a first-class type
├── HyperlaneLogging (.h)           # log categories: LogGalaxySim, LogShip, …
└── FHyperlaneSaveSchema            # versioned save structs

Simulation
├── UGalaxySubsystem                # authoritative owner, sim tick driver
│   ├── FGalaxyClock
│   ├── FTravelNetwork              # systems + hyperlanes graph
│   └── (Phase 3+: FEconomyModel, FFactionModel)
└── Data definitions
    ├── UStarSystemDef, UPlanetDef, UHyperlaneDef
    └── (Phase 3+: UFactionDef, UCommodityDef, UShipClassDef)

Gameplay
├── AShipPawn                       # thin; a socket for components
│   ├── UShipMovementComponent     # flight model, tuned from UShipClassDef
│   ├── UDockingComponent
│   └── (Phase 3+: UCargoComponent)
└── AStationActor
    └── UDockingPortComponent

UI (Phase 2+)
├── UGalaxyMapScreen
└── USimInspectorScreen             # debug window into the sim — a real deliverable
```

Notes:
- `AShipPawn` derives from `APawn`, **not** `ACharacter` — Character drags in
  a humanoid walking-movement component that's pure dead weight on a ship.
- The same ship components must serve player and (future) rendered-NPC ships.
  If a component needs to know "am I the player?", that's a design smell to
  catch in review.

## 8. Persistence

- One save-game model from Phase 2 on: the sim serializes its state
  (versioned structs), gameplay serializes the player's situation. Everything
  else is reconstructable and therefore *not saved*.
- Rule: if you can't decide whether something belongs in save data, it's sim
  state that should be derivable — derive it.

## 9. Blueprint Policy

C++ is the source of truth for all logic and architecture. Blueprint is used
where it genuinely improves workflow:

- **Yes:** data assets, subclassing C++ classes to bind meshes/VFX/sounds,
  UMG widget layout & animation, quick input/tuning experiments (then ported).
- **No:** system logic, sim code, anything another system depends on, math.
- Every C++ class decides deliberately what it exposes (`UPROPERTY(EditAnywhere)`,
  `BlueprintCallable`) — the Blueprint surface is an API we design, not a default.

## 10. Deferred Decisions Log

Big choices we are explicitly **not** making yet, so they don't get made by
accident. Each has a revisit trigger.

| ID | Decision                         | Current stance                        | Revisit when |
|----|----------------------------------|---------------------------------------|--------------|
| D1 | Gameplay Ability System (GAS)    | Not adopted; heavy for our Phase 1–4 needs | Phase 5 combat design |
| D2 | Physical module split            | Single module, layered folders        | Split triggers in §3 |
| D3 | Gameplay message bus             | Plain multicast delegates             | Subscriber sprawl, ~Phase 4 |
| D4 | Mass Entity for abstract sim     | Hand-rolled entity arrays first (learn the problem before the framework) | Abstract sim perf wall |
| D5 | World Partition / large worlds   | One level per star system, streamed   | Planet-scale ground content, Phase 6 |
| D6 | Multiplayer                      | Never planned; net-aware discipline only | Only by rewriting this doc |

## 11. Unreal Primer (for context on the above)

Ten-second versions of concepts this doc leans on — each gets a proper
walkthrough when we first use it:

- **UObject / GC:** Unreal C++ objects are garbage-collected. Any `UObject`
  pointer stored in a class **must** be a `UPROPERTY` `TObjectPtr<>`, or the GC
  can't see it and will free the object under you. This is the #1 new-dev crash.
- **Reflection (`UCLASS`/`UPROPERTY`/`UFUNCTION`):** macros that register your
  C++ with the engine, powering the editor UI, serialization, GC, and Blueprint.
- **Actor vs Component:** Actors are *things placed in a world*; Components are
  *behaviors attached to them*. We keep Actors thin and put logic in Components.
- **Subsystem:** engine-managed singleton with a well-defined lifetime
  (per-GameInstance, per-World, …). Our sim lives in one.
- **Module:** a C++ compilation/link unit with declared dependencies — Unreal's
  physical architecture boundary.
- **PrimaryDataAsset / Asset Manager:** the sanctioned pattern for "lots of
  game-defining data, discovered and async-loaded by ID".

---

*Changelog*
- 2026-07-01 — v1. Initial architecture (Phase 0).
