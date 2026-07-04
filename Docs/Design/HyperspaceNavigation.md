# Hyperspace Navigation — System Design

The roads, made routable. This is the deep spec for **SimulationFramework
§2.1** (Galaxy Topology & Hyperlanes): the complete data model and
architecture for plotting and travelling hyperspace courses. §2.1 states the
*responsibilities and rules*; this document defines the *classes, structures,
and relationships* that implement them, and the contract gameplay/UI will
call once Phase 2+ coding begins.

**Status:** design only. No gameplay code is written from this yet — it is the
blueprint the Phase 2 topology import and the Phase 4 course/travel systems
build against. A working prototype of the router already exists as pure JS in
`Assets/Map/galaxy_atlas.html`; this document promotes its proven concepts
(lane-stroke legs, course-change costs, a conditions overlay) into the
project's C++ `Def`/`State` architecture. Where a name here matches the
prototype, that is deliberate.

**Inherited non-negotiables** (TechnicalArchitecture §2, CLAUDE.md). Every
type below obeys these or it is a bug:

- **Simulation-only.** Everything in Parts 1–7 is plain C++ data and logic in
  the `Simulation/` layer with **no** Actor, UI, or rendering includes. The
  galaxy plots and flies courses whether or not anything renders them.
- **Def vs. State** (§1.1). Immutable authored identity is a `U*Def` data
  asset; the mutable present is an `F*State` struct owned by a model inside
  `UGalaxySubsystem`. They join by **stable `FName` ID**, never by pointer.
- **Data-driven for open sets** (rule 3). Lanes, systems, junctions, beacons,
  hazard regions are data assets + Gameplay Tags. Classifications are tags,
  **never** enums. Enums appear here only for genuinely closed, code-coupled
  sets (a leg is lane-borne or realspace-borne — there is no third kind).
- **Owned by `FTravelNetwork`.** The navigation system is not a new
  subsystem; it is the build-out of the `FTravelNetwork` sub-object that
  `UGalaxySubsystem` already composes. Gameplay reaches it only through
  validated command methods; it answers upward only via events.

---

## Part 0 — Architecture at a Glance

### 0.1 The three-layer model (the core pattern)

The single most important idea. Hyperspace data exists at three distinct
layers, and every feature in this document is an attribute added to one of
them. Confusing the layers is the mistake this section exists to prevent.

```
  AUTHORED GEOMETRY            DERIVED NAV GRAPH             LIVE CONDITIONS
  (ground truth, immutable)    (built once at load)          (mutable, saved)
  ────────────────────         ─────────────────────         ─────────────────
  UStarSystemDef  (points)  →  FNavNode[]  (snapped)     +   FNavConditions
  UHyperlaneDef   (strokes)  → FNavEdge[]  (adjacency)   +     lane/edge/system
  UGravityWellDef            → node/edge flags                 cost multipliers,
  UHazardRegionDef           → hazard tags on edges            closures, blockades,
  UNavBeaconDef              → beacon gates on edges           interdiction, hazards
  UHyperspaceJunctionDef     → FNavNode (deep-space)           beacon up/down
        │                            │                              │
        │ built by the importer      │ built by FTravelNetwork      │ mutated by
        │ (Phase 2)                  │ on subsystem Init            │ the sim tick
        ▼                            ▼                              ▼
                    ┌──────────────────────────────────────────┐
                    │ FTravelNetwork  (inside UGalaxySubsystem) │
                    │   FNavGraph        — the routable graph   │
                    │   FNavConditions   — the live overlay     │
                    │   FHyperspaceRouter— stateless service    │
                    │   FJourneyRegistry — in-flight travellers │
                    └──────────────────────────────────────────┘
                                       │  TryPlotCourse() ▲ events
                                       ▼                  │
                               GAMEPLAY  →  PRESENTATION
```

- **Layer A — Authored geometry.** What the atlas literally contains today:
  4,143 systems as `(x,y,z)` points and 123 hyperlanes as polyline *strokes*
  (`galaxy_map.json`). This is content, authored in the editor as data
  assets, **never** written at runtime, **never** saved. It answers *what
  exists and where*.
- **Layer B — Derived nav graph.** The routable `FNavGraph`: nodes and
  directed edges with integer indices, built **once** at load by snapping
  systems onto lane strokes (the atlas has no explicit edge list — the
  ~12,321 edges of Bible 01 are *derived*, not authored). This is a transient,
  cache-friendly acceleration structure. It answers *what connects to what,
  and how far*.
- **Layer C — Live conditions.** `FNavConditions`: the mutable overlay of
  per-edge/-lane/-system cost multipliers, closures, blockades, interdiction
  fields, active hazards, and beacon status. Owned by `FTravelNetwork`,
  versioned, saved. It answers *what is true on the roads right now*. This is
  the layer the war writes to.

Design consequence: **new hyperspace features are almost always a new
attribute on Layer A that lowers into a flag on Layer B and/or a record on
Layer C — not a new system.** Blockades, interdiction, and unexplored space
(Part 9) are all Layer-C overlays. This is the same discipline §2.1 states for
blockades ("close edges, not systems") generalized to everything dynamic.

### 0.2 Ownership & call direction

`FTravelNetwork` is the sole owner of all three layers at runtime. It exposes:

- **Downward-facing commands** — gameplay calls `UGalaxySubsystem`
  validated methods (`TryPlotCourse`, `TryBeginJourney`, `TryRecalculate`);
  the network never mutates gameplay.
- **Upward-facing events** — multicast delegates (`OnLaneStatusChanged`,
  `OnCourseInvalidated`, `OnJourneyAdvanced`); the network never knows who
  listened. UI and NPC AI subscribe identically.

The **same command path** serves the player's nav computer and an NPC
freighter's routing (non-negotiable rule 2). If any type below needs to know
"am I the player?", that is a design smell to catch in review.

---

## Part 1 — Nodes

A **node** is any point where a ship can exist in realspace and a hyperspace
leg can begin or end: a star system, a deep-space junction, or a nav-beacon
station. Nodes are the vertices of `FNavGraph`.

### 1.1 `UStarSystemDef` — the places (extends existing)

Already defined (§2.2, TechArch §7) as the system's identity, bodies, and
Active-LOD level reference. Navigation **adds** a nav profile block; it does
not redefine the asset.

```cpp
// UStarSystemDef  (Simulation/Data) — navigation additions only
FName        SystemId;            // "System.Corellia" — the universal join key
FVector      GalacticCoords;      // atlas (x,y,z), map units; the routing metric space
FGameplayTag Region;              // "Nav.Region.CoreWorlds" (allegiance-neutral geography)
FName        SectorId;            // override namespace only (Bible 01 §3)

// Nav profile (drives snapping, leg breaks, safety)
FGravityProfile   GravityProfile; // mass shadow this system projects (see 3.1)
bool              bMandatoryReversion; // busy/hazardous systems force a leg break here
FGameplayTagContainer NavTags;    // "Nav.Node.System", "Nav.Facility.Refuel",
                                  // "Nav.Facility.Astromech", "Nav.Beacon.Anchored"…
TArray<FName>     LocalBeacons;   // NavBeacon ids servicing this node
```

`GalacticCoords` is authoritative: **all distance, direction, and snapping
math is done in this space**, so travel time is grounded in real galactic
geometry, not authored fiat. `bMandatoryReversion` is how "you always drop out
at Coruscant" is expressed as data, not code.

### 1.2 `UHyperspaceJunctionDef` — deep-space nodes (new)

Not every leg break happens at a system. A **junction** is an authored node in
empty space where lanes cross, where a well forces reversion, or where a route
famously kinks (a "waypoint in the black"). It is a first-class node with no
planet, no market, and usually no facilities.

```cpp
// UHyperspaceJunctionDef  (Simulation/Data)
FName        JunctionId;          // "Junction.EriaduApproach"
FVector      GalacticCoords;
FGameplayTagContainer NavTags;    // "Nav.Node.Junction", "Nav.Node.LaneCrossing"…
FGravityProfile   GravityProfile; // usually empty; nonzero for anomaly junctions
TArray<FName>     ConnectsLanes;  // lanes that meet/transfer here
bool              bChartedByDefault; // false → discovered content (Part 9)
```

**Decision (DR-35):** junctions are authored *only where a break is
mechanically forced or narratively named* — the router discovers ordinary
optimal break points itself (§0.1). Junctions are content, not a per-crossing
work list; the 736-sector map's "never a work list" rule (Bible 01) applies
here too. Open question **OQ-9** tracks whether v1 ships any non-system
junctions at all, or defers every leg break to systems.

### 1.3 `UNavBeaconDef` — the infrastructure (new)

A **nav beacon** is authored infrastructure that makes a stretch of lane
*usable, faster to calculate, or safer*. Beacons are how "who paved the road"
becomes a live, attackable fact: a beacon has an owner and can be destroyed,
which downgrades or closes the lane it anchors (Layer C).

```cpp
// UNavBeaconDef  (Simulation/Data)
FName        BeaconId;            // "Beacon.Foerost"
FVector      GalacticCoords;
FGameplayTag Class;               // "Nav.Beacon.Class.Superlane"  (Major-lane relay)
                                  // "Nav.Beacon.Class.Regional"   (Minor-lane marker)
                                  // "Nav.Beacon.Class.Picket"     (frontier/uncharted)
FName        AnchorsLane;         // lane whose usability this beacon gates (or NAME_None)
float        CalcTimeReductionPct;// astrogation-time discount granted when up (0..1)
float        SafetyBonus;         // additive to safety rating of gated edges
FName        OwnerFactionId;      // who maintains it — the attack surface
```

At load, each beacon lowers into a **gate** on the edges it anchors (Layer B):
those edges carry a `RequiresBeacon` reference. At runtime `FNavConditions`
holds the beacon's up/down status (Layer C); a downed beacon closes or
penalizes its edges without any beacon-specific code in the router — it reads
the same cost multiplier a blockade uses.

### 1.4 The node relationship, summarized

| Node kind | Def | Has facilities | Typical role in a course |
|---|---|---|---|
| Star system | `UStarSystemDef` | Yes (refuel, astro) | Origin, destination, waypoint, forced reversion |
| Junction | `UHyperspaceJunctionDef` | Rare | Lane transfer, well-forced break, named waypoint |
| Beacon | `UNavBeaconDef` | Sometimes | Lane enabler; a node you route *past*, occasionally *to* |

All three become homogeneous `FNavNode`s in the graph (Part 6); their `Def`
kind is remembered by tag so gameplay can treat a "drop out to refuel" node
differently from "transfer lanes here."

---

## Part 2 — Edges (Hyperlanes)

An **edge** is a directed, traversable hop between two nodes. Edges are
*derived* (Layer B) from authored lane strokes (Layer A) plus off-lane
realspace connections. This is the heart of the graph.

### 2.1 `UHyperlaneDef` — the strokes (extends existing)

The authored lane. In the atlas a lane is `{ name, major, segments }` where
`segments` is a set of polyline strokes in coordinate space. Navigation
formalizes the lane's identity, class, and the **rules that ride on it**.

```cpp
// UHyperlaneDef  (Simulation/Data)
FName        LaneId;              // "Lane.CorellianRun"
FText        DisplayName;
FGameplayTag Class;               // "Nav.Lane.Class.Major" | ".Minor" | ".Local"
TArray<FPolyline> Strokes;        // authored geometry; the curve legs follow
float        BaseSpeedMult;       // class speed factor (Major fastest); tuning, not fiat
float        TrafficCapacity;     // assumed throughput — feeds piracy/patrol AI (§2.1)
FGameplayTagContainer LaneTags;   // "Nav.Lane.Superhighway", "Nav.Lane.SpiceRun"…

FLaneRestriction Restriction;     // who/what may ride this lane (2.3)
FName        RequiresBeacon;      // beacon that must be up for this lane to route (or None)
FGameplayTag DiscoveryState;      // "Nav.Charted" | "Nav.Uncharted" | "Nav.Rumored" (Part 9)
```

`Class` is the content mapping of Bible 01 §4.1 (**Major** 12 / **Minor** 110 /
**Local** ~12,100). It is a **tag**, so a future "Ancient" or "Corporate-toll"
lane class is data, never an enum edit.

### 2.2 `FPolyline` and the derived `FNavEdge`

```cpp
// FPolyline — one authored stroke of a lane (Layer A)
TArray<FVector> Points;           // ordered galactic coords

// FNavEdge — one derived directed hop (Layer B, struct-of-arrays in FNavGraph)
int32   FromNode, ToNode;         // indices into FNavGraph.Nodes
int32   LaneIndex;                // owning lane, or -1 = realspace (off-lane) edge
FVector Direction;                // unit vector From→To, for course-change (turn) math
float   BaseTravelTime;           // realspace distance / lane speed factor, at drive class 1.0
float   BaseFuelUnits;            // distance-proportional fuel at reference consumption
uint16  EdgeFlags;                // WithinGravityWell | ThroughHazard | RequiresBeacon…
```

Edges are stored **struct-of-arrays** (parallel `TArray`s keyed by edge index),
not as `UObject`s — 12,321 edges must be cache-friendly for repeated
pathfinding. `LaneIndex == -1` marks a **realspace edge**: an off-lane hop the
importer adds so isolated systems can still join the network (the prototype's
`OFFLANE_MULT` makes these deliberately expensive, so courses prefer paved
lanes). This preserves the Bible 01 truth that cutting a superhighway reroutes
trade onto slow locals *rather than stopping it*.

### 2.3 `FLaneRestriction` — lane restrictions

What a lane will and won't carry. Read by the router as hard filters (illegal →
edge unusable) or soft costs (allowed but penalized).

```cpp
// FLaneRestriction  (Simulation)
float        MaxHyperdriveClass;  // slowest drive the lane tolerates (0 = none)
FGameplayTag MinSizeClass;        // "Ship.Size.Strike".. some locals bar capitals
FGameplayTagContainer RequiredPermissions; // "Perm.Republic.Military", "Perm.HuttToll.Paid"
FGameplayTag InspectionPolicy;    // "Nav.Policy.Inspect.Contraband" (smuggling hook, §2.4)
float        TollUnits;           // base transit toll (currency), 0 if free
```

Restrictions are **capability gates, not scripted gates**: a ship either
satisfies the tags or it doesn't, and the *same* filter runs for player and
NPC. Inspection/toll policy is the diegetic hook that makes smuggling a career
(§2.1) — it is a cost term, never a minigame flag.

### 2.4 The import derivation (Layer A → Layer B)

Documented because the edge graph is *built*, not authored, and the build
rules are load-bearing. The Phase 2 importer:

1. Loads every `UStarSystemDef`/junction as an `FNavNode` at its coords.
2. For each `UHyperlaneDef` stroke, **snaps** nearby nodes onto the stroke
   (prototype `SNAP_MAX ≈ 250–400` map units) and emits ordered lane edges
   between consecutive snapped nodes — a node "on" the Corellian Run gets
   Corellian-Run edges to its lane neighbours.
3. Adds sparse **realspace edges** (`LaneIndex = -1`) so no charted node is
   unreachable, gated by a max off-lane distance.
4. Bakes `BaseTravelTime`/`BaseFuelUnits`/`EdgeFlags` from geometry, gravity
   wells (Part 3), and hazard regions (Part 4).

The result is deterministic from the authored data — **the graph is never
hand-edited** (Bible 01 standing rule). New lanes/systems re-run the import;
they are data, not code.

---

## Part 3 — Volumes: Wells & Hazards

Some navigation facts are not points or lines but *regions of space*. Volumes
are authored (Layer A), baked into edge flags at import (Layer B), and can be
switched on/off at runtime (Layer C, for the dynamic ones).

### 3.1 Gravity wells — `FGravityProfile` & `UGravityWellDef`

A **mass shadow**: hyperspace cannot be safely entered or exited inside a
sufficiently deep gravity well, and travelling *through* one forces reversion
to realspace. This is the physical law that makes interdiction (Part 9)
possible and makes some systems mandatory drop-outs.

```cpp
// FGravityProfile — a well projected by a body/system/junction (Layer A)
float ShadowRadius;               // map units within which hyperspace is denied
float Depth;                      // 0..1 severity; scales forced-reversion certainty

// UGravityWellDef — a standalone well not tied to a system (e.g. a black hole,
// the Maw, an uncharted mass) (Simulation/Data)
FName        WellId;
FVector      GalacticCoords;
FGravityProfile Profile;
FGameplayTag Kind;                // "Nav.Well.Stellar" | ".BlackHole" | ".Anomaly"
bool         bChartedByDefault;   // uncharted wells are how ships vanish (Part 9)
```

At import, any edge passing within a well's `ShadowRadius` is flagged
`WithinGravityWell` and **cannot be a leg interior** — the leg must break at the
well boundary (a forced reversion). This is one code path shared by natural
wells and artificial interdiction; Part 9 adds the interdiction case as a
runtime well with no new mechanism (**DR-38**).

### 3.2 `UHazardRegionDef` — hazardous regions

A **hazard region** is a volume that makes travel through it slower, riskier,
or forbidden: nebulae, ion storms, gravitic anomalies, minefields, radiation
bands, Kessel-style shortcuts (a hazard that also *saves time* is just a hazard
with a negative time term). Hazards are the primary input to route **safety
ratings** (Part 8).

```cpp
// UHazardRegionDef  (Simulation/Data)
FName        HazardId;            // "Hazard.MawCluster"
FConvexVolume Volume;             // authored shape in galactic coords
FGameplayTag Kind;                // "Nav.Hazard.Nebula" | ".IonStorm" | ".Anomaly"
                                  // | ".Minefield" | ".Radiation"
float        SafetyPenalty;       // subtracted from edge safety rating (Part 8)
float        TimeMult;            // >1 slower, <1 a shortcut
float        FuelMult;            // weaving/shielding fuel surcharge
bool         bDynamic;            // true → toggled/moved at runtime (Layer C)
FGameplayTagContainer Effects;    // "Nav.Effect.SensorBlind", "Nav.Effect.CommsDark"…
```

**Static** hazards (`bDynamic=false`, e.g. a permanent nebula) bake straight
into `EdgeFlags`+cost at import. **Dynamic** hazards (`bDynamic=true`, e.g. a
travelling ion storm, a freshly seeded minefield) live in `FNavConditions` as
overlay records the sim can add, move, and clear — the graph itself never
changes, only the multipliers over it (§0.1). `Effects` tags let a hazard do
more than cost time: a `SensorBlind` nebula degrades the information system
(§2.10), which is why smugglers love them.

---

## Part 4 — The Runtime Graph & Conditions

`FTravelNetwork` composes the runtime home of Layers B and C. These types have
**no authored assets** — they are built from, and overlay on top of, the defs.

### 4.1 `FNavGraph` — the routable structure (Layer B)

```cpp
// FNavGraph  (Simulation) — built once in FTravelNetwork::BuildGraph()
TArray<FNavNode>        Nodes;        // vertices (systems, junctions, beacons)
// Edges as struct-of-arrays, indexed 0..EdgeCount:
TArray<int32>  EdgeFrom, EdgeTo, EdgeLane;
TArray<FVector> EdgeDir;
TArray<float>  EdgeBaseTime, EdgeBaseFuel;
TArray<uint16> EdgeFlags;
TArray<TArray<int32>> Adjacency;      // node -> outgoing edge indices
TMap<FName,int32>      NodeIndex;     // "System.Corellia" -> node index (the join)
TMap<FName,int32>      LaneIndex;     // "Lane.CorellianRun" -> lane index
```

```cpp
// FNavNode  (Simulation)
FName        NodeId;              // joins back to the Def
FVector      Coords;
FGameplayTagContainer NavTags;    // copied from Def for fast router queries
uint8        Kind;               // System | Junction | Beacon (closed set → enum ok)
```

The graph is **transient**: rebuilt deterministically from defs on load,
**never saved** (saving it would duplicate the source of truth and rot). It is
the only place struct-of-arrays is used, and for one reason: pathfinding over
12k edges runs hot.

### 4.2 `FNavConditions` — the live overlay (Layer C, saved)

The mutable state of the roads. This *is* the war's effect on travel, and the
only navigation state that persists.

```cpp
// FNavConditions  (Simulation) — owned by FTravelNetwork, versioned, saved
TMap<int32,float>  LaneCostMult;    // lane index -> multiplier (>1 avoid, INF closed)
TMap<int32,float>  EdgeCostMult;    // edge index -> multiplier (fine-grained)
TMap<int32,float>  SystemCostMult;  // node index -> penalty on edges into it
TArray<FBlockade>          Blockades;      // Part 9
TArray<FInterdictionField> Interdictions;  // Part 9
TArray<FDynamicHazard>     ActiveHazards;  // moving/temporary hazards (Part 3.2)
TMap<FName,bool>   BeaconUp;        // beacon id -> operational (Part 1.3)
TMap<int32,FGameplayTag> EdgeStatus; // Open/Contested/Blockaded/Closed (§2.1)
```

`FNavConditions` is a plain, unit-testable struct — the sim mutates it, the
router reads it, the inspector renders it. An **empty** conditions object means
peacetime: nothing closed, nothing penalized (matching the prototype's stated
default). Every wartime fact — a blockade, a contested stretch, a downed
beacon, a minefield — is an entry here, and removing the entry relaxes the
galaxy with zero bespoke cleanup (§2.1's blockade rule, generalized).

### 4.3 Status events (upward-facing)

`FTravelNetwork` fires these when `FNavConditions` changes; UI and AI subscribe
identically (no god-view — observers act on news-speed knowledge, §2.10):

| Event | Fires when | Consumed by |
|---|---|---|
| `OnLaneStatusChanged(LaneId, NewStatus)` | blockade/closure toggles a lane | map UI, trader AI reroute |
| `OnHazardChanged(HazardId, State)` | dynamic hazard added/moved/cleared | map UI, safety recompute |
| `OnBeaconStatusChanged(BeaconId, bUp)` | beacon destroyed/restored | map UI, lane routability |
| `OnCourseInvalidated(JourneyId, Reason)` | an in-flight course's next leg became illegal | that traveller's controller |

---

## Part 5 — The Course Model

The centerpiece. A **course** is a computed plan to get a specific ship from A
to B under a specific cost preference at a specific moment. It is a sequence of
**legs** joined at realspace **waypoints**. This is where "single direct jump"
and "multi-jump journey with intermediate exits" both live as one structure.

### 5.1 Definitions: leg, waypoint, course

- **Leg** — one continuous hyperspace jump: engage drive at an entry node,
  disengage at an exit node, no realspace in between. A leg is **either** a
  *lane leg* (rides one lane's stroke, cheap and fast) **or** a *realspace
  leg* (an off-lane hop, dear). There is no third kind → this is the one
  legitimate enum in the model.
- **Waypoint** — a realspace node *between* legs: where the ship reverts, and
  can recalculate, refuel, wait out a hazard, run an errand, or be intercepted.
  Waypoints are why multi-jump journeys exist and why they are *interruptible*.
- **Course** — the ordered legs + the derived aggregate metrics (Part 8). A
  **direct single-jump** course is simply a course with one lane leg and no
  intermediate waypoints; nothing special-cases it.

```cpp
// ECourseLegKind — the one closed set
enum class ECourseLegKind : uint8 { Lane, Realspace };

// FCourseLeg  (Simulation)
ECourseLegKind Kind;
FName        EntryNodeId, ExitNodeId;
FName        LaneId;              // valid iff Kind==Lane
TArray<int32> EdgePath;           // graph edges this leg traverses (one lane's run)
FRouteMetrics Metrics;            // time, fuel, safety for THIS leg (Part 8)
FGameplayTag ExitReason;          // "Nav.Break.Transfer" | ".GravityWell" | ".FuelRange"
                                  // | ".Waypoint" | ".Destination"
```

```cpp
// FCourseWaypoint  (Simulation)
FName        NodeId;
FGameplayTag Purpose;            // "Nav.Stop.Recalc" | ".Refuel" | ".Errand" | ".Transfer"
bool         bPlayerRequested;   // authored stop vs. router-forced break
```

```cpp
// FCourse  (Simulation) — the plan; transient, recomputable, not saved as-is
FName        CourseId;           // handle for in-flight tracking
FName        OriginId, DestinationId;
TArray<FCourseLeg>      Legs;
TArray<FCourseWaypoint> Waypoints;   // Legs.Num()-1 interior stops (+ optional player stops)
FRouteMetrics Aggregate;             // whole-course totals (Part 8)
FRouteCostProfile ProfileUsed;       // which preference produced this (5.3)
FGalacticDateTime PlottedAt;         // conditions snapshot time — courses staledate
```

### 5.2 Why legs break (the leg-boundary rules)

The router closes a leg and opens a waypoint for exactly these reasons — each a
tag in `FCourseLeg::ExitReason`, so gameplay/UI can explain the stop:

| Reason | Trigger | Player-visible meaning |
|---|---|---|
| `Transfer` | course change: switching lanes / a sharp bend | "change heading at X" |
| `GravityWell` | edge enters a mass shadow (Part 3.1) | "forced reversion — mass ahead" |
| `FuelRange` | cumulative leg fuel would exceed tank (Part 8.2) | "refuel stop required" |
| `Waypoint` | a `bPlayerRequested` stop | "your planned stop" |
| `Destination` | arrival | end of course |

The first three are *emergent* from the graph and the ship — the router finds
them; they are not authored per-route. This is the Bible 01 §4.3 chokepoint
philosophy applied to a single ship's plan: breaks are **discovered, not
declared**.

### 5.3 `FRouteCostProfile` — one router, many preferences

The router is one algorithm; the *preference* is data. A cost profile weights
the terms of an edge's cost, exactly as §2.1 requires ("one router, many cost
functions") and as the prototype implements (`TURN_COST`, `TRANSFER_COST`,
`OFFLANE_MULT`). It is **not** a ship stat — the same ship plots a fast course
or a safe course by swapping the profile.

```cpp
// FRouteCostProfile  (Simulation)
float WeightTime;        // haste
float WeightFuel;        // economy
float WeightSafety;      // avoid contested/hazard/interdiction-prone edges
float WeightToll;        // avoid paid lanes (a warship sets this ~0)
float WeightInspection;  // avoid inspection policy (a smuggler sets this high)
float TurnCost;          // per course-change penalty (prototype ≈ 220)
float TransferCost;      // per lane-switch flat fee   (prototype ≈ 320)
float OfflaneMult;       // realspace-edge multiplier   (prototype ≈ 2.5)
bool  bAllowUncharted;   // may the router consider Rumored/Uncharted edges? (Part 9)
```

Named archetype profiles are **data assets** (`URouteProfileDef`), not code:
`Profile.Courier` (time-dominant), `Profile.BulkHauler` (fuel+toll), `Profile.
Smuggler` (inspection+safety, offlane-tolerant), `Profile.Warship` (time,
toll≈0). Adding a preference is a new asset, never a router change.

### 5.4 `FCourseRequest` / `FCourseResult` — the query contract

```cpp
// FCourseRequest  (Simulation) — everything the router needs, nothing it doesn't
FName        OriginId, DestinationId;
FHyperdriveProfile Drive;         // the ship's drive (Part 8.1) — class, range, consumption
FRouteCostProfile  Profile;       // preference (5.3)
TArray<FName> RequiredWaypoints;  // player-authored intermediate stops (ordered)
FGameplayTagContainer ShipCaps;   // size/permission tags, checked vs FLaneRestriction
FGalacticDateTime  AsOf;          // conditions snapshot to plot against
```

```cpp
// ECourseStatus — result outcomes (closed set)
enum class ECourseStatus : uint8 {
  Ok, NoRouteExists, BlockedByRestriction, OutOfFuelRange,
  DestinationUnknown, OriginInGravityWell
};

// FCourseResult  (Simulation)
ECourseStatus Status;
FCourse       Course;             // valid iff Status==Ok
FName         FailingNodeOrLane;  // for diagnostics/UI ("no legal drive class for Lane.X")
```

A failure is **data with a reason**, never an exception — the nav computer UI
turns `BlockedByRestriction` + `FailingNodeOrLane` into "your hyperdrive is too
slow for the Kessel Run." The inspector renders the same result verbatim.

---

## Part 6 — The Router

`FHyperspaceRouter` is a **stateless service**: it holds no state, takes an
`FNavGraph` + `FNavConditions` + `FCourseRequest`, and returns an
`FCourseResult`. Statelessness is deliberate — the router can be called from
the sim tick, a headless commandlet test (Phase 3), or the UI thread's plot
preview without owning anything.

### 6.1 Algorithm (documented, not yet coded)

Phase-aware least-cost search — Dijkstra/A\* over **(edge, arrival-phase)
states**, not bare nodes, because course-change cost depends on the direction
you *arrived* from (the prototype's proven approach). Per-edge cost:

```
cost(edge | arrived_via) =
    edge.BaseTravelTime * driveClassMult * laneClassMult   * Profile.WeightTime
  + edge.BaseFuel       * driveConsumption                 * Profile.WeightFuel
  + (1 - edgeSafety)                                        * Profile.WeightSafety
  + edgeToll                                                * Profile.WeightToll
  + inspectionRisk                                          * Profile.WeightInspection
  + courseChangePenalty(arrived_via, edge)   // TurnCost·(1−dir·dir) [+ TransferCost]
  × conditionsMult(edge)                     // NAV_CONDITIONS: contested/closed/hazard
```

- **Hard filters first** (illegal → edge skipped entirely): closed status,
  failed `FLaneRestriction`, gravity-well interior, downed required beacon,
  uncharted edge when `!bAllowUncharted`.
- **Leg assembly:** after the search yields a node path, contiguous same-lane
  edges collapse into one lane leg; a lane change, a well boundary, a fuel-range
  cut, or a required waypoint closes a leg and opens a waypoint (§5.2).
- **Required waypoints** are honored by routing `Origin → W1 → … → Dest` as
  chained searches, each seeded fresh (no arrival phase across a realspace
  stop).

### 6.2 Recalculation & staledating

A course is a **snapshot** against `PlottedAt` conditions. When conditions
change under an in-flight ship (`OnLaneStatusChanged` on a lane in a *future*
leg), `FTravelNetwork` fires `OnCourseInvalidated`; the traveller calls
`TryRecalculate(JourneyId)`, which re-plots **from the next waypoint**, never
mid-leg (you cannot change your mind inside hyperspace — you drop out first).
This is exactly why the course is legged: the waypoints are the *only* legal
decision points, in fiction and in code.

### 6.3 Cost & scale

One search touches ≤ 12,321 edges × a small phase factor; A\* with the
Euclidean galactic-coords heuristic prunes hard because the metric space is the
real map. Target: a full cross-galaxy plot is sub-millisecond at Abstract LOD,
so NPC traders re-plot freely when blockades move (§2.1's emergent rerouting
depends on this being cheap). No route caching in v1 — recomputation is cheaper
than invalidation bookkeeping; revisit only if profiling says so.

---

## Part 7 — Journeys (in-flight state)

A course is a *plan*; a **journey** is a ship *executing* one. Journeys are the
only navigation state besides `FNavConditions` that persists — a save mid-trip
must resume the same trip.

```cpp
// FJourneyState  (Simulation) — one traveller mid-course; saved
FName        JourneyId;
FName        TravellerId;         // ship/fleet entity id (sim entity, not an Actor)
FCourse      Course;              // the plan being executed
int32        CurrentLeg;          // index into Course.Legs
float        LegProgress;         // 0..1 along the current leg
FGalacticDateTime Departed, ETA;  // sim-clock times (never float-accumulated, rule 4)
FGameplayTag Phase;               // "Nav.Phase.InHyperspace" | ".AtWaypoint" | ".Arrived"

// FJourneyRegistry  (Simulation) — owned by FTravelNetwork
TArray<FJourneyState> Active;     // every in-flight traveller, player and NPC alike
```

Journeys advance on the **sim clock** (`FGalaxyClock`, fixed timestep), not
per rendered frame — a distant NPC freighter's "Corellia→Duro, 62%" (rule 5)
*is* a `LegProgress` value, promoted to a rendered Actor only at Active LOD.
`OnJourneyAdvanced` fires at leg boundaries so the map UI and the news system
(§2.10) can react (arrivals, overdue ships, ships that jumped into a fresh
blockade). The player's own journey uses the **same** struct — the nav computer
is a *view* onto `FJourneyState`, not a parallel system.

---

## Part 8 — Derived Metadata

The numbers a course carries. All are **computed**, never authored on the
course — authored inputs live on defs; the course holds only results.

### 8.1 Hyperdrive class modifiers — `FHyperdriveProfile`

The ship's contribution to travel. Derived from the ship's `UShipClassDef`
Performance group (Bible 05 §2, `hyperdriveRating`) — navigation reads it, does
not duplicate it.

```cpp
// FHyperdriveProfile  (Simulation) — read from the ship, passed into a request
float HyperdriveClass;    // canon scale: Class 0.5 (fast) … 1.0 … 4.0 (barge). Lower = faster.
float BackupClass;        // limp-home drive if primary fails (0 = none)
float FuelConsumption;    // units per base-fuel unit of edge
float TankUnits;          // fuel capacity → sets max leg length before a FuelRange break
float CalcSpeed;          // astrogation compute rate (nav computer quality; OQ-11)
```

`driveClassMult = HyperdriveClass` — a Class 0.5 drive halves lane transit
time, a Class 2.0 doubles it (canon/Legends convention: the multiplier *is* the
class number). This is the whole of "ship hyperdrive class modifiers": one
multiplier on `BaseTravelTime`, applied per edge in §6.1. `TankUnits` vs
accumulated `BaseFuelUnits` is what forces `FuelRange` leg breaks (§5.2).

### 8.2 Travel time & fuel — `FRouteMetrics`

```cpp
// FRouteMetrics  (Simulation) — attached per leg AND aggregated per course
FGalacticDateTime Duration;  // Σ edge BaseTravelTime · driveClassMult · laneClassMult · condMult
float        FuelUnits;      // Σ edge BaseFuel · consumption · hazardFuelMult
float        SafetyRating;   // 0..1 composite (8.3)
int32        CourseChanges;  // number of transfers/turns — a legibility metric for UI
float        TollTotal;      // Σ lane tolls on the course (currency)
```

Time is **real** (Pillar 5) and expressed in `FGalacticDateTime`, never
accumulated floats (rule 4) — a course from the Core to the Rim is *days*, and
that duration is the reason blockades, logistics, and news delay all matter.
Fuel ties to the economy: fuel is `Commodity.Fuel` (Bible 06), so a long course
is a *cost*, and a refuel `FuelRange` break is a market interaction, not a
gauge.

### 8.3 Safety rating — the composite

A single 0..1 legibility score per edge, leg, and course. **Computed, layered,
and explainable** — never an authored "danger stat":

```
edgeSafety = clamp01(
    laneClassBase            // Major paved 0.9 … Local 0.4  (Bible 01 §4.1)
  + beaconSafetyBonus        // active beacons raise it (Part 1.3)
  + patrolControlBonus       // controlling-faction patrol strength (§2.2/§2.8)
  − hazardPenalty            // Σ overlapping hazard regions (Part 3.2)
  − blockadeProximity        // near a blockaded/contested edge (Part 9)
  − interdictionRisk )       // interdiction-prone stretch (Part 9)
```

Course safety is the **min** across its legs (a chain is as safe as its worst
jump), surfaced to UI as a rating *and* a breakdown ("safe except the Ison leg:
CIS-contested"). Because every term is a live Layer-C value, safety **moves
with the war** for free — the same freshness the price/news systems have.

---

## Part 9 — The Dynamic Layer (Future Features, Designed-In Now)

These are the features §2.1 promises "grow by new edge attributes and cost
terms, not new systems." Each is specified as a **Layer-C overlay record** so
the graph, router, and course model above need **zero** changes to support
them. This is the payoff of the three-layer model.

### 9.1 Blockades — `FBlockade`

A fleet holding an edge (§2.1: *close edges, not systems*).

```cpp
// FBlockade  (Simulation) — a record in FNavConditions.Blockades
int32   EdgeIndex;               // or LaneIndex for a whole-lane blockade
FName   BlockadingFactionId;
float   Strength;                // porousness: high → EdgeCostMult = INF; low → steep but passable
FName   AssignedFleetId;         // the fleet whose presence sustains it (§2.8)
```

Effect is pure overlay: sets `EdgeStatus = Blockaded` and a large
`EdgeCostMult`. Routing avoids it, prices diverge, news spreads, the fleet burns
supply — **all emergent, no blockade code in the router**. Remove the fleet →
remove the record → the galaxy relaxes. A *porous* blockade (low strength) is a
steep cost, not `INF`, so a bold smuggler with an offlane-tolerant profile can
still run it — texture for free.

### 9.2 Interdiction fields — `FInterdictionField`

An artificial gravity well projected by an Interdictor-class ship or a
gravitic mine: it *creates* a mass shadow where none exists, forcing reversion
and trapping ships mid-course.

```cpp
// FInterdictionField  (Simulation) — a record in FNavConditions.Interdictions
FVector GalacticCoords;
FGravityProfile Profile;         // SAME type as a natural well (Part 3.1)
FName   SourceEntityId;          // the projecting ship/mine
bool    bAmbush;                 // suppresses advance news (§2.10) — the trap's whole point
```

Because it reuses `FGravityProfile`, an interdiction field runs the **exact
same forced-reversion path** as a natural gravity well (DR-38) — the router
already breaks legs at well boundaries, so it breaks them here too. The only new
behavior is *dynamism*: the field can appear on a stretch a ship is already
committed to, firing `OnCourseInvalidated` and dropping the traveller to
realspace at the field edge — the canonical hyperspace ambush, implemented as a
moving well plus an existing event.

### 9.3 Unexplored space & discovery

Wild Space and the Unknown Regions exist "as geography and rumor only" (Bible
01 §2). Navigation models this as a **discovery state** on lanes/nodes/wells,
not a separate map:

```cpp
// DiscoveryState tag on defs + a per-observer known-set (ties §2.11 Knowledge)
"Nav.Charted"    // in every ship's astrogation database
"Nav.Rumored"    // known to exist, geometry uncertain — routable only speculatively
"Nav.Uncharted"  // not in the database — invisible to the router unless bAllowUncharted
```

- The router ignores non-`Charted` edges unless the request's `bAllowUncharted`
  is set (an explorer's profile), and even then prices them with a large
  uncertainty penalty — you *can* jump blind, but the safety rating says don't.
- **Discovery** flips a node/edge's known-state in the observer's knowledge set
  (the map "grows as you do", §2.11) — surveying a rumored lane, buying charts,
  or surviving a blind jump. The galaxy's *ground-truth* graph already contains
  these edges; discovery is a per-observer visibility mask over it, so exploring
  reveals a consistent galaxy rather than generating one.
- Uncharted **gravity wells** (Part 3.1, `bChartedByDefault=false`) are the
  danger: a blind jump can hit a mass nobody logged. This is why Wild Space
  eats ships — modeled, not scripted.

**OQ-10** tracks the discovery *mechanics* (survey action, chart economy,
skill/nav-computer dependence) for Phase 5+; the *data model* for it is
complete here.

---

## Part 10 — Persistence, LOD & UE Integration

### 10.1 What saves, what rebuilds

| Layer | Type | Persistence |
|---|---|---|
| A — authored geometry | `U*Def` assets | Never saved; shipped content |
| B — nav graph | `FNavGraph` | Never saved; **rebuilt** from defs on load (deterministic) |
| C — live conditions | `FNavConditions` | **Saved**, versioned in `FHyperlaneSaveSchema` |
| — in-flight | `FJourneyState[]` | **Saved** (resume mid-trip) |
| — per-observer knowledge | discovery masks | Saved (part of §2.11 Knowledge) |

Courses (`FCourse`) are **not** saved standalone — they are recomputable from
`(request, conditions)`. A saved *journey* carries its course as a value so a
resumed trip is byte-stable even if conditions shifted while saved.

### 10.2 LOD (matches §2.1)

- **Abstract** — graph math only. Every NPC journey is `FNavGraph` +
  `FJourneyState`; 10,000 travellers are arrays, not Actors.
- **Nearby** — in-system traffic near the player becomes lightweight tracked
  entities; journeys near the player gain positional detail.
- **Active** — ships on lanes near the player render, and can be interdicted,
  hailed, attacked. Only here does navigation touch the Gameplay/Presentation
  layers, and only through views onto sim state.

### 10.3 UE integration plan (Phase 2+ coding)

Documented so the eventual code has no architectural questions to reopen:

1. **Defs are `UPrimaryDataAsset` subclasses** registered with
   `UHyperlaneAssetManager` as primary asset types (`StarSystem`, `Hyperlane`,
   `NavBeacon`, `HazardRegion`, `GravityWell`, `Junction`, `RouteProfile`).
   Async-loaded, cooked, discoverable by tag scan — never hardcoded lists
   (rule 3).
2. **Classifications are Gameplay Tags** under a `Nav.*` root (Part 11), so
   lane classes, hazard kinds, break reasons, and discovery states extend as
   data.
3. **The graph builds in `FTravelNetwork::Initialize()`**, invoked by
   `UGalaxySubsystem` on `OnWorldBeginPlay`, from the loaded def set — the one
   place Layer A becomes Layer B.
4. **Gameplay calls validated commands** on `UGalaxySubsystem`, never the
   network directly: `TryPlotCourse(FCourseRequest) → FCourseResult`,
   `TryBeginJourney(FCourse) → FName JourneyId`, `TryRecalculate(FName)`. The
   player's nav-computer component and an NPC trader's AI call the identical
   method (rule 2).
5. **Presentation subscribes to events** (§4.3, Part 7) and **queries**
   read-only snapshots; it never holds refs into `FNavConditions` or
   `FNavGraph` internals (rule 2, banned patterns).
6. **DataTables only for tuning tuples** — lane-class speed factors,
   size-class fuel constants, safety-term weights — never for the open sets,
   which are assets (rule 3).
7. **Inspector view ships with the system** (Definition of Done): a
   `USimInspectorScreen` tab renders `FNavConditions`, the active journeys, and
   a plotted course's leg/metric breakdown — the debug window is a real
   deliverable, in the same change as the logic, under a `LogGalaxyNav`
   category.

---

## Part 11 — Gameplay Tag Schema (`Nav.*`)

The open-set vocabulary. New entries are additive; nothing here is an enum.

```
Nav.Region.{DeepCore,CoreWorlds,Colonies,InnerRim,ExpansionRegion,
            MidRim,OuterRim,HuttSpace,WildSpace,UnknownRegions}   // geography (Bible 01)
Nav.Node.{System,Junction,Beacon,LaneCrossing}
Nav.Lane.Class.{Major,Minor,Local}
Nav.Lane.{Superhighway,SpiceRun,TollRoad,...}                    // lane character
Nav.Beacon.Class.{Superlane,Regional,Picket}
Nav.Well.{Stellar,BlackHole,Anomaly}
Nav.Hazard.{Nebula,IonStorm,Anomaly,Minefield,Radiation}
Nav.Effect.{SensorBlind,CommsDark,DriveStrain}                   // hazard side-effects
Nav.Break.{Transfer,GravityWell,FuelRange,Waypoint,Destination}  // leg-exit reasons
Nav.Stop.{Recalc,Refuel,Errand,Transfer}                        // waypoint purposes
Nav.Policy.Inspect.{Contraband,Toll,None}                       // lane restriction policy
Nav.Phase.{InHyperspace,AtWaypoint,Arrived}
Nav.{Charted,Rumored,Uncharted}                                 // discovery state
Nav.Facility.{Refuel,Astromech,Repair,Charthouse}               // node services
```

---

## Part 12 — Class & Relationship Reference (master catalog)

Every type in the framework, one line each. Layer: **A** authored def, **B**
derived graph, **C** live state, **X** transient query/result, **S** service.

| Type | Layer | Owns / owned by | One-line responsibility |
|---|---|---|---|
| `UStarSystemDef` | A | AssetManager | A system's identity, coords, gravity profile, facilities |
| `UHyperspaceJunctionDef` | A | AssetManager | A deep-space node where lanes cross or a well forces a break |
| `UNavBeaconDef` | A | AssetManager | Infrastructure that gates/speeds/secures a lane; ownable, attackable |
| `UHyperlaneDef` | A | AssetManager | A lane: strokes, class, speed, restrictions, beacon/discovery gates |
| `UGravityWellDef` | A | AssetManager | A standalone mass shadow that forces reversion |
| `UHazardRegionDef` | A | AssetManager | A volume that slows/endangers/blocks travel; static or dynamic |
| `URouteProfileDef` | A | AssetManager | A named cost-preference archetype (Courier, Smuggler…) |
| `FPolyline` | A | `UHyperlaneDef` | One authored lane stroke (ordered coords) |
| `FGravityProfile` | A | system/junction/well/interdiction | Shadow radius + depth of a mass shadow |
| `FLaneRestriction` | A | `UHyperlaneDef` | Drive-class/size/permission/toll/inspection gates on a lane |
| `FNavGraph` | B | `FTravelNetwork` | The routable node/edge structure, built from defs at load |
| `FNavNode` | B | `FNavGraph` | A graph vertex (system/junction/beacon) with coords + tags |
| `FNavEdge` (SoA) | B | `FNavGraph` | A directed hop: from/to, lane or realspace, base time/fuel/flags |
| `FNavConditions` | C | `FTravelNetwork` | The live overlay: cost mults, closures, blockades, hazards, beacons |
| `FBlockade` | C | `FNavConditions` | A fleet closing an edge/lane; a cost multiplier + status |
| `FInterdictionField` | C | `FNavConditions` | A runtime gravity well (Interdictor/mine) forcing reversion |
| `FDynamicHazard` | C | `FNavConditions` | A moving/temporary hazard region instance |
| `FRouteCostProfile` | X | `FCourseRequest` | The weighted preference the router optimizes for |
| `FHyperdriveProfile` | X | `FCourseRequest` | The ship's drive class, range, consumption, calc speed |
| `FCourseRequest` | X | caller | The full plot query: endpoints, drive, profile, waypoints, caps, time |
| `FCourseResult` | X | router → caller | Status + course (or a reasoned failure) |
| `FCourse` | X | result / journey | The plan: ordered legs + waypoints + aggregate metrics |
| `FCourseLeg` | X | `FCourse` | One continuous jump (lane or realspace) with its metrics |
| `FCourseWaypoint` | X | `FCourse` | A realspace stop between legs (recalc/refuel/errand/transfer) |
| `FRouteMetrics` | X | leg & course | Computed time, fuel, safety, course-changes, tolls |
| `FHyperspaceRouter` | S | `FTravelNetwork` | Stateless least-cost course plotter over graph+conditions |
| `FJourneyState` | C | `FJourneyRegistry` | One ship executing a course; current leg, progress, ETA — saved |
| `FJourneyRegistry` | C | `FTravelNetwork` | All in-flight travellers, player and NPC alike |
| `FTravelNetwork` | — | `UGalaxySubsystem` | Owns all three layers + router + journeys; the system's front door |

**Relationship spine, in one sentence each:**

- `UGalaxySubsystem` **owns** `FTravelNetwork`, which **owns** `FNavGraph`
  (built from the `U*Def` set), `FNavConditions`, `FHyperspaceRouter`, and
  `FJourneyRegistry`.
- `UHyperlaneDef` strokes **snap** `UStarSystemDef`/junction nodes into
  `FNavEdge`s; wells and hazard regions **flag** those edges at import.
- A caller hands `FTravelNetwork` an `FCourseRequest`; the `FHyperspaceRouter`
  reads `FNavGraph` filtered by `FNavConditions` and returns an
  `FCourseResult` wrapping an `FCourse` of `FCourseLeg`s joined by
  `FCourseWaypoint`s.
- Beginning a course spawns an `FJourneyState`; the sim clock advances it;
  condition changes fire events that may invalidate and re-plot it.
- Blockades, interdiction fields, and discovery are **records/masks on
  `FNavConditions`** — no new systems, exactly as §2.1 promised.

---

## Register Entries

Proposed for `DecisionRegister.md` (DR-33…38 continue the sequence; OQ-9…11
open). Added there in the same change.

| ID | Decision | Choice | Why |
|---|---|---|---|
| DR-33 | Nav data layering | **Three layers: authored geometry (defs) → derived graph (built at load, never saved) → live conditions (overlay, saved)** | Features become attributes on a layer, not new systems; the graph never rots because it's rebuilt from source |
| DR-34 | Course structure | **Course = ordered legs joined at realspace waypoints; a leg is lane-borne or realspace-borne; direct jump = one-leg course** | One structure serves single and multi-jump; waypoints are the only legal recalc/interrupt points, in fiction and code |
| DR-35 | Leg breaks & junctions | **Breaks (transfer/well/fuel) are discovered by the router; junctions authored only where forced or named** | Bible 01 §4.3 "discovered not declared" applied per-ship; no per-crossing authoring |
| DR-36 | Routing preference | **One stateless router; preference is a weighted `FRouteCostProfile` data asset** | §2.1's "one router, many cost functions"; adding a preference is an asset, not code |
| DR-37 | Dynamic roads | **Blockades, contested stretches, dynamic hazards, downed beacons are all `FNavConditions` overlay records — never bespoke systems** | Generalizes §2.1's blockade rule; removing a record relaxes the galaxy with zero cleanup |
| DR-38 | Interdiction = gravity well | **Interdiction fields reuse `FGravityProfile` and the natural-well forced-reversion path; only dynamism is new** | The hyperspace-ambush feature costs one overlay record + an existing event, no new mechanic |

| ID | Open question | Resolve by |
|---|---|---|
| OQ-9 | Does v1 author any non-system junctions, or defer every leg break to systems? | Phase 2 topology import |
| OQ-10 | Discovery *mechanics* — survey action, chart economy, nav-computer/skill dependence (ties OQ-7) | Phase 5+ |
| OQ-11 | Does astrogation *calculation* consume in-game time (nav-computer quality × distance × beacon coverage), or is jump prep instant in v1? | Phase 4 travel system |

---

## Pillar Check

- **1 — The Galaxy Lives:** every NPC and cargo ship routes and flies on the
  same graph, router, and journey model as the player, on the sim clock, with
  no player in the loop. Conditions are standing pressure, never scheduled.
  *Serves.*
- **2 — Systems Over Scripted Events:** chokepoints, leg breaks, reroutes, and
  ambush-viable stretches are all *emergent* from geometry + conditions +
  cost profile — none authored per-route. *Serves.*
- **4/6 — Modular, reusable:** one router serves every agent via data
  profiles; one gravity-well path serves natural wells and interdiction; one
  course structure serves single and multi-jump; blockade/hazard/beacon are
  one overlay type. *Serves.*
- **5 — Immersion / Danger:** travel time is real days in `FGalacticDateTime`;
  fuel is a real commodity; safety is a live composite; you drop out to decide,
  never mid-jump. Distance *means* something. *Serves.*
- **8 — Design for Expansion:** blockades, interdiction, and unexplored space
  are specified now as overlays/masks needing **zero** changes to the graph,
  router, or course model. New lanes/systems are data + a re-import. *Serves.*

## Scalability Check

- **Authoring cost is flat:** the edge graph is *derived* by snapping, not
  hand-built — 4,143 systems and 122 named lanes produce ~12,321 edges with no
  per-edge authoring. New content is more of the same data. *Passes.*
- **Runtime cost is bounded:** struct-of-arrays graph + A\* on real
  coordinates keeps a cross-galaxy plot sub-millisecond; 10,000 journeys are
  arrays advanced by the clock, promoted to Actors only at Active LOD.
  *Passes.*
- **State cost is minimal:** only `FNavConditions`, journeys, and per-observer
  discovery masks persist; the graph rebuilds from defs. Save size tracks *war
  activity*, not galaxy size. *Passes.*
- **Watch:** the snapping importer's `SNAP_MAX` threshold and realspace-edge
  density are the two dials that decide graph connectivity and size — a bad
  value silently over- or under-connects the galaxy. Import must emit a
  connectivity report (isolated nodes, edge count, degree histogram) as a
  Phase 2 deliverable. *Watched.*

---

*Changelog*
- 2026-07-04 — v1. Full data model and architecture for hyperspace
  navigation: three-layer model (geometry/graph/conditions), nodes (systems,
  junctions, beacons), edges + restrictions, volumes (wells, hazards), the
  course/leg/waypoint model, the stateless router + cost profiles, derived
  metadata (time, fuel, hyperdrive class, safety), journeys, and the dynamic
  layer (blockades, interdiction, unexplored space). Registers DR-33…38,
  OQ-9…11. Formalizes the `galaxy_atlas.html` router prototype into the C++
  Def/State architecture.
