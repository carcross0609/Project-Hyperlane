# Coding Standards

Baseline: the [Epic C++ Coding Standard](https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine).
This document doesn't repeat it — it highlights what matters most for this
project and adds our own rules. When in doubt, do what the engine source does.

## Naming

- Epic prefixes, always: `A`ctor classes, `U`Object classes, `F`structs/plain
  classes, `E`num types, `I`nterfaces, `T`emplates, `b`Booleans.
- PascalCase for types, functions, and variables (Unreal style — not
  camelCase): `CurrentThrottle`, not `currentThrottle`.
- Booleans read as assertions: `bIsDocked`, `bBlockadeActive`.
- Functions are verbs: `ResolveBattle()`, `TryExecuteTrade()`. Functions
  returning bool ask a question: `CanDockAt(...)`. `Try` prefix = may fail,
  returns/outputs a result.
- Star Wars names are welcome in gameplay and content
  (`ULightsaberComponent`, `Faction.Hutts`); deep sim internals stay generic
  (`FEconomyModel`, not `FGalacticCreditModel`) — it costs nothing and keeps
  systems reusable (Pillar 6).
- No project prefix on class names (no `UHLShipPawn`); if an engine name
  collision forces one, use full `Hyperlane`.

## Files & Structure

- One public class per header. Filename == class name without prefix
  (`ShipPawn.h` for `AShipPawn`).
- `#pragma once`; includes minimal; **forward-declare in headers, include in
  .cpp**. Header bloat is compile-time debt we refuse from day one.
- Every file starts with `// Copyright (c) 2026 Carson Crossno. Personal,
  non-commercial project.`
- Folder placement follows the layer map in TechnicalArchitecture.md §3.
  A file that's hard to place is a design question — stop and think.

## Unreal-Specific Rules (the crash-preventers)

- Every `UObject*` member is `UPROPERTY()` `TObjectPtr<T>`. No exceptions —
  raw `UObject` pointers are invisible to the garbage collector.
- Non-owning, possibly-outlived references: `TWeakObjectPtr<T>`. Check before use.
- `check(X)` for "impossible by construction" (crashes loudly in dev),
  `ensure(X)` for "shouldn't happen but recoverable" (logs once, continues),
  `if` for expected runtime conditions. Never validate user/data input with `check`.
- Prefer subsystem access over static singletons; never cache subsystem
  pointers across level loads unless the subsystem's lifetime guarantees it.
- Tick is opt-in, not default: constructors set
  `PrimaryActorTick.bCanEverTick = false` unless the class earns its tick.
  Sim work goes on the sim clock, not on actor ticks.
- Delegates bound with `AddDynamic`/`AddUObject` are unbound in
  `EndPlay`/`Deinitialize` if the listener can die before the broadcaster.

## Modern C++ Usage

- `auto` only when the type is obvious on the same line (iterators, casts,
  lambdas) — Epic's rule, and it keeps code reviewable.
- `const` correctness everywhere: methods, parameters, locals. A non-const
  method is a claim that it mutates state — make it true.
- References for required parameters, pointers for optional ones.
- No `new`/`delete` for UObjects (always `NewObject`/`SpawnActor`); engine
  containers (`TArray`, `TMap`, `FString`) over STL in engine-facing code.
- No magic numbers: named constants, config, or (preferably) data assets.

## Logging & Diagnostics

- One log category per system (`LogGalaxySim`, `LogShip`, `LogEconomy`),
  declared in `Core/HyperlaneLogging.h`.
- Log state *transitions* at `Log`, detail at `Verbose`. A silent system is
  an undebuggable system; a spammy one gets muted and becomes silent.
- Every sim system ships with its inspector/debug view in the same change
  (TechnicalArchitecture.md §1.6).

## Comments & Documentation

- Comments explain **why**, never narrate what the code visibly does.
- Public headers get doc comments on the class and any non-obvious method —
  written for you-in-two-years, our most important teammate.
- `// TODO(phase-N): reason` — TODOs carry a phase and a reason, and get
  swept at each phase boundary. Undated TODOs are deleted on sight.

## Definition of Done (every change)

1. Compiles with zero new warnings.
2. Follows the layer rules (self-review the diff for upward dependencies).
3. Tunable values live in data, not literals.
4. Log category and (for sim systems) inspector coverage exist.
5. Self-reviewed as a diff before commit — read it like a hostile senior
   engineer would.
