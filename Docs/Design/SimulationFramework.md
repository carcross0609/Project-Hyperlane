# Simulation Framework — Game Design Document

How the galaxy functions. This document defines the simulation systems that
make Pillar 1 ("The Galaxy Lives Without the Player") true: factions,
governments, economies, trade, military logistics, hyperlanes, planets,
fleets, NPCs, and the player's place among them.

**Scope.** This is the *framework* document: it defines each system's
responsibilities, data model, and contracts with other systems. Deep tuning
(price curve constants, AI utility weights) belongs in per-system design docs
written when each system is built, per its Roadmap phase. This doc does not
contain code and does not change TechnicalArchitecture.md — it fits inside it.

**Reading order.** Part I defines framework-wide rules every system obeys.
Part II specifies the systems. Part III shows how they compose, persist, and
map onto the Roadmap.

---

# Part I — Framework Rules

These rules apply to every simulation system. A system that needs an
exception documents it here or it's a bug (CorePillars: Using the Pillars).

## 1.1 Definition vs. State

Every simulation noun splits into two halves:

- **Definition (`U*Def`, a data asset):** immutable identity and capability.
  What Corellia *is*, what a CR90 corvette *can do*, what the Republic
  *values*. Authored in the editor, loaded via Asset Manager, never written
  at runtime, never saved.
- **State (`F*State`, a plain struct):** the mutable present. Corellia's
  current stockpiles, this fleet's position and supply, the Republic's
  treasury. Owned by a model object inside `UGalaxySubsystem`, versioned,
  serialized in saves.

The two halves link by **stable ID** (an `FName`, e.g. `System.Corellia`,
`Faction.Republic`), never by pointer. IDs are the universal join key across
systems, saves, logs, and the inspector. Nothing in the simulation holds an
object pointer to anything in another system.

## 1.2 Two Tiers of Galaxy Data

The imported atlas (`Assets/Map/galaxy_map.json`) contains **4,143 star
systems** and a routing graph of **12,321 edges over 123 named hyperlanes**.
We will never hand-author 4,143 data assets, and we don't need to:

- **Catalog systems** — every system in the atlas gets a lightweight
  generated record: ID, name, position, region, grid, lane connections. The
  simulation runs over *all* of them: fleets travel through them, lanes route
  through them, wars claim them.
- **Authored systems** — a growing subset (10–15 in Phase 2) additionally
  gets a full `UStarSystemDef` asset: planets, stations, markets, population,
  landable content. Only authored systems have economies and are visitable
  at full fidelity.

A catalog system can be *promoted* to authored at any time by creating its
asset; nothing else changes. This is the content growth path for the life of
the project.

## 1.3 The Clock and Cadences

The sim advances on `FGalaxyClock` in fixed timesteps (TechnicalArchitecture
§4.2). Systems do not all think at the same rate — each subscribes to a
**cadence**, and the clock buckets work so no single tick pays for the whole
galaxy:

| Cadence | Game-time | Typical subscribers |
|---|---|---|
| Fast | continuous (every sim tick) | travel progress, combat resolution in progress |
| Hourly | each game hour | market price recalculation, local NPC decisions |
| Daily | each game day | production/consumption, fleet supply draw, news propagation |
| Weekly | each game week | faction strategic AI, government policy review, population drift |

All cadences are tunables, not constants. Time compression (hyperspace
journeys, debug fast-forward) multiplies the clock; cadence subscribers are
oblivious — they just fire more often per real second.

## 1.4 Simulation LOD Applies to Every System

The `Active / Nearby / Abstract` LOD ladder (TechnicalArchitecture §4.3) is
not just for NPC ships — every system in this document defines its behavior
per LOD. The rule of thumb:

- **Abstract** is the *canonical* implementation and is built first. It must
  produce believable, explainable outcomes on its own (Pillar 1's headless
  test runs entirely at Abstract).
- **Nearby/Active** are *refinements* that spend fidelity where the player is
  looking. Promoting an entity must never change what was already true about
  it; demoting must summarize honestly (a fleet that lost 3 fighters while
  Active is a fleet of N−3 when Abstract).

## 1.5 Events Are the Only Way Up (and Sideways-ish)

Systems communicate results by broadcasting typed sim events
(`FSimEvent`: ID, timestamp, location, subject IDs, payload). Three kinds of
consumers, in order:

1. **Other sim systems** subscribe to react (economy hears
   `Event.Lane.Blockaded` and reroutes trade).
2. **The news/information system** (§2.10) turns events into diegetic,
   location-aware knowledge.
3. **Presentation** spawns effects, UI, wreckage — the sim never knows.

Sim events are also the **explanation trail**: the inspector and headless
tests answer "why did this change?" by replaying the event log. Every state
mutation of consequence emits an event or it didn't happen.

## 1.6 Commands Are the Only Way In

All writes into sim state — player *and* NPC — go through validated command
methods on the owning model (`TryExecuteTrade`, `TryIssueFleetOrder`,
`TrySetPolicy`). Commands return typed results (accepted / rejected-because).
There is exactly one door per verb; who is knocking is a parameter, not a
code path (net-aware discipline, TechnicalArchitecture §1.5).

---

# Part II — The Systems

## 2.1 Galaxy Topology & Hyperlanes

**Responsibility:** the shape of space — what connects to what, how long
travel takes, and who controls the roads. Owned by `FTravelNetwork`.

### Data model

| Definition (per lane / generated from atlas) | State (runtime) |
|---|---|
| Lane ID, name (e.g. `Lane.CorellianRun`) | Per-edge status: `Open / Contested / Blockaded / Closed` |
| Edge list: (systemA, systemB, base transit time) | Blockading faction + strength, if any |
| Class: `Major` (named routes) / `Minor` / `Local` | Traffic level (rolling volume, feeds piracy & patrol AI) |
| Region/sector membership | Toll/inspection policy in force (from controlling government) |

The 12,321-edge graph from the atlas is the ground truth. Lane **class**
affects transit speed, traffic capacity assumptions, and how famous the route
is (news/rumor reach).

### Rules

- **Travel time is real** (Pillar 5). Transit time = path length over edge
  base times × ship drive rating × lane-class modifier. No teleportation, no
  exceptions — including NPC fleets and cargo. Distance is why logistics,
  blockades, and information delay all *mean* something.
- **Routing** is shortest-time pathfinding over open edges, with per-agent
  weights: a smuggler weights inspection risk, a bulk hauler weights toll
  cost, a warship ignores tolls. One router, many cost functions.
- **Blockades close edges, not systems.** A blockade is a fleet assignment
  (§2.8) holding a specific edge. Its effects are entirely emergent: routing
  avoids it, prices diverge on both sides, news spreads, the blockading
  fleet consumes supply. Remove the fleet and everything relaxes — no
  blockade-specific logic anywhere else.
- **Interdiction & inspection:** controlled edges can impose cargo
  inspection (contraband law, §2.4) and tolls. This is the hook that makes
  smuggling a career rather than a minigame.

**Per LOD:** Abstract = graph math only. Nearby = in-system traffic becomes
lightweight tracked entities. Active = ships on lanes near the player render
and can be interacted with (interdicted, hailed, attacked).

**Grows by:** new lanes/edges are data. Later features (unstable routes,
uncharted-route discovery, seasonal Kessel-style shortcuts) are new edge
attributes and router cost terms, not new systems.

## 2.2 Star Systems & Planets

**Responsibility:** the places. What exists at each node of the network.

### Data model

`UStarSystemDef` (authored tier): system ID, display info, contained bodies,
station list, ambient traffic profile, level reference for Active LOD.

`UPlanetDef` — identity and *capacity*, never current state:

| Field group | Contents |
|---|---|
| Identity | ID, name, system, tags (`World.Type.Urban`, `World.Climate.Desert`) |
| Population capacity | Species mix, max population class, culture tags |
| Economic capacity | Production slots: what industries *can* run here (`Industry.Shipbuilding`, `Industry.Agri`) and at what scale |
| Political defaults | Default sovereign faction, default local government type, stability baseline |
| Content | Landable locations, station defs (Phase 6: interiors) |

`FPlanetState` — the living planet:

| Field group | Contents |
|---|---|
| Population | Current population level, growth trend, unrest |
| Economy | Active industries and output levels, stockpiles (§2.5), development level |
| Politics | Current sovereign faction, current local government, stability, control strength |
| Military | Garrison strength, fortification level, supply reserve |

**Rules**

- Planets are the **anchors of everything**: markets live on them (and their
  stations), factions fight over them, fleets resupply at them, populations
  generate demand from them. A system with no authored planets still exists
  (catalog tier) but hosts no economy.
- **Stations are planets-lite:** same state shape, smaller numbers, plus a
  parent (planet, lane edge, or deep space). Trade stations on lane edges
  are how empty space gets an economy.
- Planet state changes only via commands issued by owning systems (economy
  ticks production; faction AI changes garrisons; invasions change
  sovereignty) — the planet itself has no AI; it is a ledger.

**Grows by:** new tags and production slots are data. Phase 6 ground content
attaches to `Content` without touching sim fields.

## 2.3 Factions

**Responsibility:** the galaxy's actors — entities with resources, goals,
and memory. Owned by `FFactionModel`.

### Data model

`UFactionDef`: ID, identity (name, iconography tags), **doctrine weights**
(expansionist / mercantile / isolationist / predatory — the personality
vector that tunes the strategic AI), government type (§2.4), starting
holdings and relations, ship/troop roster (which `UShipClassDef`s it fields).

`FFactionState`:

| Field group | Contents |
|---|---|
| Resources | Treasury (credits), materiel stockpile, manpower pool, influence |
| Holdings | Controlled systems/planets (by ID), claimed-but-uncontrolled |
| Relations | Per-faction disposition: `Allied / Friendly / Neutral / Rival / Hostile / AtWar`, plus a scalar drift value and treaty flags |
| Memory | Grievances and favors ledger (bounded, decaying) — including toward the player, who is simply another entry |
| Strategy | Current strategic goals and their assigned operations (§ below) |

### Strategic AI

Utility-based, three altitudes, evaluated on the weekly cadence:

1. **Goals** (persistent, few): *Secure the Rimma Trade Route*, *Break the
   Republic's hold on sector X*, *Get rich*. Scored from doctrine weights ×
   situation (threats, opportunities, resources).
2. **Operations** (per goal, seasonal): blockade edge E, invade planet P,
   fortify system S, escort convoys on lane L, raid region R. Each operation
   is a budget: credits + materiel + fleets assigned.
3. **Orders** (per fleet/asset, daily): move, hold, patrol, engage, resupply
   — consumed by the fleet system (§2.8).

The AI only acts through the same command layer as everyone else. It cannot
cheat: an invasion it can't supply (§2.7) fails the same way a player's
would.

**Scale note:** major factions (Republic, CIS, Hutts) run full strategic AI.
Minor factions (a planet's pirate clan, a regional guild) run a reduced
single-goal loop. Which factions exist, and at which tier, is data.

**Grows by:** new doctrines are weight vectors; new operation types register
into the operation set; espionage/diplomacy verbs land as new command types
against existing relation/memory state.

## 2.4 Governments

**Responsibility:** how holdings are ruled — the policy layer that turns
faction control into concrete local rules. Deliberately separate from
factions: the Republic and the CIS are factions; "how Corellia is actually
governed this year" is government.

### Data model

`UGovernmentTypeDef` (e.g. `Gov.RepublicSenate`, `Gov.HuttKajidic`,
`Gov.MilitaryOccupation`, `Gov.CorporateCharter`): default policy vector,
legitimacy model (what keeps stability up: prosperity, fear, tradition),
transition rules (what it degrades into under occupation or collapse).

**Policy vector** (the knobs, all data, applied per-holding in
`FPlanetState.Politics`):

| Policy | Feeds into |
|---|---|
| Tax & tariff rates | Faction treasury; market prices; smuggling incentive |
| Contraband list (tag query, e.g. `Commodity.Contraband.*`) | Lane inspections; black-market margins |
| Patrol funding | Security level on nearby edges; piracy suppression |
| Conscription rate | Faction manpower; planetary unrest |
| Trade openness | Which factions' traders may dock; embargo enforcement |

### Rules

- **Sovereignty vs. administration.** A faction *owns* a planet; a
  government type *runs* it. Conquest usually swaps sovereign while
  degrading government to `MilitaryOccupation` (stability penalty, higher
  extraction) until stabilized — occupation is expensive, which is why wars
  of conquest strain economies (§2.7).
- **Stability is the output.** Prosperity, policy harshness, culture match,
  war exposure, and unrest feed a stability score. Low stability throttles
  production and tax yield; at the floor it emits revolt events — a new
  minor faction is instantiated from data templates. Revolts are the
  framework's pressure valve, not scripted events (Pillar 2).
- Policy changes are faction AI commands on the weekly cadence, and later a
  player verb (Phase 6+ politics career) through the same door.

**Grows by:** new government types and policies are data. The Phase 6+
politics career is UI over existing policy commands, not a new system.

## 2.5 Economy

**Responsibility:** production, consumption, and price discovery. Owned by
`FEconomyModel`. This is the first living system built (Phase 3) and the
one every other system leans on.

### Data model

`UCommodityDef`: ID, tags (`Commodity.Fuel`, `Commodity.Contraband.Spice`),
base value, volume per unit, production recipe (inputs → outputs, per
industry tag).

Markets: every authored planet/station carries per-commodity market state —
**stock, target stock, local price, recent volume**.

### Rules

- **Goods are real.** Every unit of tibanna gas exists in exactly one place:
  a market's stock, a ship's hold, or a fleet's supply train. Production
  creates, consumption destroys, everything between is *transport* (§2.6).
  No spreadsheet teleportation. This conservation is what makes blockades,
  piracy, and war shortages arithmetically honest rather than scripted.
- **Prices are local.** Price = base value × curve(stock / target stock) ×
  policy modifiers (tariffs, embargo scarcity). No galactic price — the
  spread between local prices *is* the trading game, and information about
  prices travels at news speed (§2.10), not instantly.
- **Production runs on the daily cadence:** industries consume input
  stockpiles and population labor, emit output to local stock. Population
  generates baseline consumption (food, fuel, goods) scaled by development.
  Shortfalls raise prices and, if sustained, unrest (§2.4) — the economy
  and politics loop closes by construction.
- **Sinks and sources are budgeted.** Credits enter via faction spending and
  production value, leave via taxes, tolls, and upkeep. v0 does not need a
  perfect monetary model, but every source/sink is logged from day one so
  inflation is a diagnosable bug, not a mystery.

**Per LOD:** the entire economy is Abstract by design — markets are state,
not places. Active LOD merely *renders* it (market UI, cargo loading).

**Grows by:** new commodities/recipes are data (the market system doesn't
know it's trading tibanna — Pillar 6). Futures, contracts, and player-owned
production are new command types over the same market state.

## 2.6 Trade & Civilian Logistics

**Responsibility:** moving real goods across the real network — the NPC
traders that make prices converge and lanes matter.

### Rules

- **Traders are agents, not effects.** An abstract trader is a sim entity:
  ship class (capacity, speed, from `UShipClassDef`), home region, credits,
  a current plan (buy X at A, haul to B, sell). They perceive prices with
  news-speed information (§2.10) — they can be *wrong*, arrive late, get
  scooped. Their collective error is market texture.
- **Route choice is the router (§2.1) with a profit cost function**: margin
  per unit ÷ round-trip time, discounted by risk (piracy on route, war
  zones, inspection if hauling contraband). Traders avoiding a lane *is*
  the signal patrols and pirates read.
- **Population scales with prosperity.** Trader counts per region drift on
  the weekly cadence toward what trade volume supports. Prosperous peace →
  more freighters; war → fewer, pricier, escort-hungry.
- The **player is one trader among many**: same market commands, same
  router, same cargo rules. An exploitable price gap the player finds is
  found by NPC traders too, and arbitraged away (Roadmap Phase 3 exit
  criterion) — the framework guarantees this because NPCs and player share
  one market.

**Per LOD:** Abstract traders are structs advancing along edges. Nearby,
they become trackable contacts; Active, they are rendered ships whose cargo
is really aboard (piracy on a rendered freighter debits the same ledger).

**Grows by:** convoys and escort contracts (Phase 5) compose traders with
fleet protection postures; player-owned trade companies are trader agents
whose owner ID happens to be the player.

## 2.7 Military Logistics

**Responsibility:** the constraint that makes war strategic — armies march
on supply, and supply moves on hyperlanes like everything else.

### Rules

- **Supply is a commodity** (`Commodity.MilitarySupply`, produced by
  industry from inputs like food/fuel/materiel). It obeys §2.5 conservation:
  produced on worlds, stockpiled in depots (planet military reserve),
  hauled by dedicated logistics convoys — which are fleets (§2.8) with
  cargo, using the same lanes, visible to the same piracy and interdiction.
- **Fleets draw supply daily.** In friendly territory with an open route to
  a depot, they draw automatically (draw amount scales with distance —
  long lines cost more). Cut off, they consume their onboard reserve, then
  suffer attrition: readiness drops, then strength. **Sieges and deep
  offensives are therefore won by lane control**, which is why blockades
  (§2.1) are the fundamental military verb of this galaxy.
- **Operations budget supply up front.** The faction AI (§2.3) plans an
  invasion as: strength required *and* supply line required. An operation
  whose route gets cut doesn't instantly fail — it starves on the daily
  cadence, visibly, generating news (§2.10) the player can read, exploit,
  or relieve.
- **War economy closes the loop:** heavy operations drain materiel and
  treasury, spike demand for supply inputs on producer worlds (prices move,
  traders respond, §2.6), and conscript manpower (unrest, §2.4). Peace is
  cheap; war is a burn rate. Factions that overreach lose — to arithmetic,
  not to a script.

**Grows by:** Phase 6 ground invasions add a troop-transport requirement to
the same convoy/supply machinery; nothing structural changes.

## 2.8 Fleets

**Responsibility:** the unit of military presence and force resolution.

### Data model

`FFleetState`: fleet ID, owner faction, **composition** (counts per
`UShipClassDef` ID — never individual ship objects at Abstract), location
(system ID, or edge + progress), current order, posture
(`Passive / Defensive / Aggressive / Evasive`), readiness, onboard supply,
derived strength (cached from composition × readiness).

`UShipClassDef` (shared with gameplay): role tags, speed, capacity,
strength contribution, upkeep cost.

### Rules

- **Orders come from faction AI** (or the player, later, through the same
  command): move, patrol (system set or edge), blockade edge, escort
  (convoy ID), engage, invade (planet), resupply. A fleet executes exactly
  one order; complex behavior lives in the operation layer above (§2.3).
- **Abstract combat resolution (Phase 4):** when hostile fleets meet
  (co-located, postures/relations imply battle), resolve on the fast
  cadence over multiple ticks — not instantly — using strength, readiness,
  supply, composition matchups, and local modifiers. Multi-tick resolution
  matters: battles have duration, so news can spread, reinforcements can
  arrive, and the player can *stumble into* one.
- Outcomes debit composition (ships are destroyed, not respawned — losses
  hit faction materiel replacement pipelines), emit battle events, leave
  salvage state at the location.
- **Phase 5 escalation:** a battle at Active LOD promotes to real combat
  around the player. The abstract resolver still governs everything outside
  render range, and the two agree on totals when the battle demotes (§1.4).

**Grows by:** named capital ships and admirals (Phase 5+) are optional
attachments to composition entries — flavor and modifiers on top of the
same resolution math. Player fleet command (Phase 6+) is order-issuing
permission, not a new system.

## 2.9 NPC Behavior

**Responsibility:** the people. Everything from a cantina patron to a rival
trader with a grudge — at a cost the simulation can afford.

### Three tiers

| Tier | Representation | Count | Examples |
|---|---|---|---|
| **Cohorts** | Statistical aggregates in planet/region state | Everyone | Populations, labor, generic traffic, crowd density |
| **Agents** | Sim entities with role template + state | Thousands | Traders (§2.6), pirate bands, patrol captains, bounty hunters |
| **Persons** | Agents + identity, memory, relationships | Dozens–hundreds | Named rivals, faction leaders, recurring contacts |

- **Cohorts** feed the economy (labor, demand) and politics (unrest); they
  never act individually.
- **Agents** run role templates: a utility loop over role-appropriate goals
  (trader: profit; pirate: prey-rich/patrol-thin hunting grounds; patrol:
  cover funded edges). Agents perceive through the information system
  (§2.10) — they act on what they'd plausibly know, not on god-view state.
  Piracy emerges where the *data* says it pays (Roadmap Phase 5 exit
  criterion) because pirate agents literally read traffic and patrol state.
- **Persons** add persistent memory (the faction grievance ledger in
  miniature), relationships, and promotion history. Persons are *made, not
  authored*: an agent that meaningfully interacts with the player (or with
  events the player witnessed) is promoted to Person and remembered — the
  rival trader who finally hires a bounty hunter (Vision.md) is an agent
  the framework promoted, not a scripted character. Authored seed Persons
  (named leaders) are allowed as seed data only (Pillar 2).

**Per LOD:** cohorts are always Abstract. Agents promote to rendered pawns
at Active only when present where the player is. Persons additionally
persist appearance/voice/name so re-encounters land.

**Grows by:** new roles are template data. Phase 6 character-scale life
(schedules, interiors) refines Person/Agent presentation without touching
their sim representation.

## 2.10 Information & News (HoloNet)

**Responsibility:** who knows what, when. The system that makes the galaxy
feel big (Pillar 5: information is local and imperfect) and the diegetic
surface for everything the sim does.

### Rules

- Sim events (§1.5) become **news items** with a location of origin. News
  propagates along the lane network on the daily cadence — fast on Major
  lanes and HoloNet-connected worlds, slow in the Rim, not at all past
  blockades (cutting a lane cuts the *news* too, which is itself
  strategically exploitable).
- **Knowledge is per-observer.** Markets known to a trader, war news known
  to the player, prices shown in the galaxy map UI — all read through an
  observer-relative knowledge query ("what does X know, as of when"), never
  raw state. The player's UI is just an observer with a nice renderer. Out
  of date is the default state of everyone's map.
- **Rumors** are news items with degraded precision and reliability
  (numbers fuzzed, sources vague). Distance and hops degrade; paying for
  better sources (Phase 4+ verbs) sharpens.
- The player learns of sim consequences **in fiction first** — HoloNet
  feed, market boards, dock gossip (Roadmap Phase 4 exit criterion). Debug
  truth lives in the sim inspector, clearly out-of-world.

**Grows by:** propaganda and information warfare (factions injecting biased
items) and an espionage career are new producers/filters over the same
propagation substrate.

## 2.11 Player Progression

**Responsibility:** how the player grows — as *capability accumulation*,
never permission (Pillar 3). There are no levels, no XP, no class selection,
no gates. Progression is four ledgers the simulation already keeps:

| Ledger | What it is | Examples of growth |
|---|---|---|
| **Assets** | What you own | Ships, cargo, credits, property, (later) crews and companies — all sim entities owned by the player's ID |
| **Standing** | What factions/persons remember | The same relations & grievance ledger every faction/person keeps (§2.3, §2.9) — the player is one more entry, at personal scale |
| **Licenses & Ranks** | What institutions have granted | Data-defined grants (trade license, bounty accreditation, military rank, senate credential) — each unlocks *commands and access*, is revocable, and is faction-relative |
| **Knowledge** | What you've learned | Known systems, price intel freshness, route knowledge, rumor networks (§2.10) — the map literally grows as you do |

- **Careers are descriptive, not selected.** "Bounty hunter" is a label the
  game (and factions) apply to observed behavior. Every career is a lens on
  systems that already run without the player: trading (§2.5–2.6), combat
  and escort (§2.8, Phase 5), politics (§2.4, Phase 6+), and beyond. A new
  career costs UI + a few command verbs — never a new simulation.
- **Character skills** (piloting feel, ground abilities) are deliberately
  deferred to Phase 6 planning, where they must arrive as capability
  modifiers, not gates. Logged here so it isn't decided by accident.
- **Reversibility:** standing decays toward neutral, licenses lapse or get
  revoked, assets can be lost. No ledger entry is a one-way door (Pillar 3:
  any role, any time, reversibly).

**Grows by:** every new sim verb any system adds is automatically a
progression surface — grants gate verbs, standing prices them, assets scale
them, knowledge reveals them.

---

# Part III — Composition

## 3.1 System Interaction Map

Arrows are the *only* sanctioned influences. Everything flows through
commands (writes), queries (reads), and events (notifications) per §1.5–1.6.

```
                    ┌──────────────┐
                    │  FACTIONS    │  goals, budgets, relations
                    │  (§2.3)      │
                    └──┬───────┬───┘
             policy    │       │ orders + operation budgets
                       ▼       ▼
            ┌────────────┐   ┌────────────┐     supply draw   ┌──────────────┐
            │ GOVERNMENTS│   │  FLEETS    │◄─────────────────►│ MIL LOGISTICS│
            │ (§2.4)     │   │  (§2.8)    │  battles, blockades│ (§2.7)       │
            └─────┬──────┘   └─────┬──────┘                   └──────┬───────┘
   taxes, law,    │                │ lane control, presence          │ supply is
   stability      ▼                ▼                                 │ a commodity
            ┌─────────────────────────────────┐                      │
            │ TOPOLOGY & HYPERLANES (§2.1)    │◄─────────────────────┘
            │ + SYSTEMS & PLANETS (§2.2)      │
            └─────┬───────────────────┬───────┘
   production,    │                   │ routes, travel time, risk
   population     ▼                   ▼
            ┌────────────┐     ┌────────────┐
            │ ECONOMY    │◄───►│ TRADE      │   NPC AGENTS (§2.9) act inside
            │ (§2.5)     │     │ (§2.6)     │   trade, fleets, and planets
            └─────┬──────┘     └─────┬──────┘
                  │  events          │  events
                  ▼                  ▼
            ┌─────────────────────────────────┐
            │ INFORMATION / HOLONET (§2.10)   │──► every observer's knowledge,
            └─────────────────────────────────┘    incl. THE PLAYER (§2.11)
```

The player enters everywhere through the same doors: market commands, fleet
orders (later), policy verbs (later), and the observer knowledge query.

## 3.2 Roadmap Mapping

This framework is built in the order the Roadmap already commits to; no
phase needs a system from a later phase:

| Phase | Framework deliverables from this document |
|---|---|
| **2 — Galaxy of Data** | §1.1–1.2 (def/state, two-tier data), §2.1 network + travel (no blockades), §2.2 defs + minimal state |
| **3 — Living Economy** | §1.3 cadences, §2.5 economy, §2.6 traders (Abstract), §2.9 agent tier v0, §2.10 minimal (price info delay) |
| **4 — The Galaxy Moves** | §2.3 factions, §2.4 governments v0 (policy + stability), §2.7 logistics, §2.8 fleets + abstract battles, §2.1 blockades, §2.10 HoloNet, §2.11 standing ledger |
| **5 — Danger** | §2.8 Active-LOD battle escalation, §2.9 pirate/patrol/hunter roles, §2.11 licenses & combat careers |
| **6+** | §2.4 politics career, §2.7 ground invasion logistics, §2.9 Person-scale life, §2.11 character skills decision |

Each phase writes its per-system design doc (tuning, formulas, UI) before
implementation, per Pillar 7. This document is amended — with changelog —
when reality disagrees with it.

## 3.3 Persistence

Per TechnicalArchitecture §8: definitions are assets and never saved; all
`F*State` structs are versioned and serialize through the save schema.
Notable derived-not-saved data: cached fleet strength, router caches, price
history beyond the news horizon, everything at Nearby/Active LOD (rebuilt
from Abstract truth on load). The event log persists only as far back as
the news system needs; the inspector's deep history is a debug ring buffer,
not save data.

## 3.4 Pillar Check

- **1 — The Galaxy Lives:** every system's canonical form is Abstract and
  player-independent (§1.4); the headless week-diff test is answerable from
  the event trail (§1.5). *Serves.*
- **2 — Systems Over Scripted Events:** revolts, rivals, piracy, and
  shortages are all pressure-valve outputs of running systems; authored
  content is seed data only (§2.4, §2.9). *Serves.*
- **3 — Player Freedom:** progression is four reversible ledgers, careers
  are descriptive (§2.11). *Serves.*
- **4 — Everything Is Modular:** systems join only by ID, command, query,
  and event (§1.1, §1.5–1.6, §3.1). *Serves.*
- **5 — Immersion Over Convenience:** real travel time, observer-relative
  knowledge, in-fiction-first news (§2.1, §2.10). *Strained knowingly:*
  information locality risks tedium; per CorePillars this pillar loses ties
  — playtests may add convenience reads (price age indicators, trusted
  broker services) without granting omniscience.
- **6 — Reusable Systems:** markets don't know their commodities, the
  router serves traders/fleets/news alike, one relation ledger serves
  factions, persons, and the player. *Serves.*
- **7 — Quality Over Speed:** framework-first with per-phase design docs;
  known risk is *over*-engineering — each phase implements only its §3.2
  slice, and speculative fields stay out of v0 schemas. *Watched.*
- **8 — Design for Expansion:** every system section carries a "grows by"
  paragraph; open sets are tags and assets throughout. *Serves.*

---

*Changelog*
- 2026-07-02 — v1. Initial simulation framework (written during Phase 0,
  targeting Phases 2–5; galaxy atlas data already in `Assets/Map/`).
