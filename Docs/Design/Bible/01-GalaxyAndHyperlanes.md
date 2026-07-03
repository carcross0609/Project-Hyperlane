# Bible 01 — The Galaxy and Its Hyperlanes

The stage. This section defines the galaxy's geography, its political map at
tick zero (22 BBY, days after Geonosis), and the hyperlane network that makes
both of them matter. Mechanics live in SimulationFramework §2.1–§2.2; this
document is the content those mechanics run on.

**The one idea this section is built on:** the Clone Wars is a war *about
hyperlanes*. Territory in a galaxy is meaningless — nobody occupies empty
light-years. What factions actually hold, cut, tax, and die for are the
routes. Every political fact below is stated in terms of who sits on which
lane, and the war the simulation fights will be legible on the map for the
same reason.

---

## 1. The Galaxy at a Glance

Ground truth is the imported atlas (`Assets/Map/galaxy_map.json`):

- **4,143 star systems** across **11 named regions** and **736 sectors**
- **122 named hyperlanes**, of which **12 are flagged major** — the
  galactic superhighways
- **12,321 routing edges** — the full navigable graph, including unnamed
  local connections

Roughly 42% of all systems (1,729) sit in the Outer Rim — the galaxy is
mostly frontier, and the war's center of gravity is exactly there.

*(Data note: 157 systems carry region `Other` — atlas artifacts to be
re-binned into proper regions during the Phase 2 import; tracked here so it
isn't forgotten.)*

## 2. The Regions — Design Roles

Each region has a *design role*: the kind of stories, economies, and dangers
it generates. Regions are the coarsest brush of the political map and the
default tier of allegiance inheritance (§3).

| Region | Systems | Design role |
|---|---|---|
| **Deep Core** | 70 | The vault. Dense stars, treacherous navigation, few stable lanes. Politically Republic-loyal to the point of irrelevance; economically a curiosity. Late-game space: hard to reach, secrets keep well here. |
| **Core Worlds** | 386 | The engine. Coruscant, Corellia, Kuat — government, shipyards, finance, and the densest population in the galaxy. Produces manufactured goods, ships, and political power; imports food and raw everything. The war is *news* here, not weather — unless it isn't, someday. |
| **Colonies** | 239 | The old money ring. Long-settled, industrialized, loyal. Extends the Core's economy outward; the first region that would feel a real Republic defeat. |
| **Inner Rim** | 315 | The comfortable belt. Wealthy, stable, complacent — high consumption, modest production. Politically soft: loyal while loyalty is cheap. |
| **Expansion Region** | 299 | The strip mine. Heavily exploited resource extraction under corporate charters — which makes it quietly contested ground, since the same corporations back the Confederacy. Loyalty here follows the ledger. |
| **Mid Rim** | 662 | The seam. Where Republic writ starts thinning and the war's front line naturally falls. Mixed economies, mixed loyalties, heavy hyperlane traffic passing through — the classic theater for blockade-and-counterblockade. Kashyyyk, Naboo, Malastare. |
| **Outer Rim** | 1,729 | The war. Nearly half the galaxy: frontier worlds, resource prizes, weak institutions, long Republic neglect — the Confederacy's recruiting ground and industrial base (Geonosis, Raxus). Piracy, smuggling, and Huttese gravity fill every gap the war leaves. Most of the game happens here. |
| **Hutt Space** | 89 | The third power. Not neutral — *sovereign*. The Hutts sit astride the Triellus Trade Route and sell to both sides. A parallel economy (spice, slaves, salvage, information) with its own law. The war enters Hutt Space only by Hutt invitation or Hutt miscalculation. |
| **Wild Space** | 134 | The edge of the map. Charted barely, governed never. Where you go to disappear, and where things come from that nobody planned for. |
| **Unknown Regions** | 63 | The dark. No lanes, no law, no data. Exists in v1 as geography and rumor only — a deliberate blank kept for future expansion, not content to fill now. |
| *(Other)* | *157* | *Atlas artifacts — see data note above.* |

## 3. The Political Map at Tick Zero

### 3.1 How allegiance is authored (three-tier inheritance)

With 736 sectors, sector-by-sector authoring is per-system authoring in
disguise. Allegiance is therefore authored as **inheritance with overrides**
— the same pattern the Framework uses for everything open-ended:

1. **Region default** — 11 assignments. *Example: Outer Rim defaults to
   `Contested`.*
2. **Sector override** — only where a sector differs from its region;
   target ≤ 60 overrides at launch. *Example: within the contested Outer
   Rim, the Arkanis Sector is `Seceded`; Hutt Space region overrides to
   `Hutt Sovereignty`.*
3. **System exception** — hand-placed flags for individually famous
   worlds; target ≤ 40. *Example: Kamino (`Loyalist` enclave deep in
   contested space), Mandalore (`Neutral` by treaty), Felucia
   (`Contested — active front`).*

Total authored assignments: **~100** to politically paint 4,143 systems.
The moment the sim starts, faction AI (Framework §2.3) owns the map; these
tiers are *only* initial conditions and are never consulted again after
tick zero.

### 3.2 Allegiance states

Allegiance is a tag on political state, not a number:

- **`Loyalist`** — governed under the Galactic Republic.
- **`Seceded`** — formally joined the Confederacy of Independent Systems.
- **`Contested`** — claimed by both; control is whatever fleets and
  garrisons say it is (Framework §2.2 control strength). The war's fuel.
- **`Neutral`** — declared non-belligerent and *recognized* as such
  (Mandalore, Hapes Cluster, Corporate Sector). Neutrality is a stance,
  not a shield: it holds exactly as long as both sides value it.
- **`Hutt Sovereignty`** — the third-power carve-out; a bloc, not an
  absence (see §2, Hutt Space).
- **`Unaligned`** — nobody's claim worth stating: Wild Space, deep
  frontier, dead systems. Distinct from Neutral: nobody asked.

### 3.3 The starting picture, painted

The opening map, stated as lane geography per this section's thesis:

- **The Republic holds the center and the trunks.** Core, Deep Core,
  Colonies, and Inner Rim are `Loyalist`; the Core-adjacent stretches of
  all twelve superhighways are Republic-patrolled. The Republic's problem
  is *reach*: its power projects down long lanes into regions that stopped
  believing in it years ago.
- **The Confederacy holds the rim ends of the lanes.** `Seceded` strength
  concentrates where the superhighways terminate in the Outer Rim — the
  Foundry worlds of the Tion/Raxus reach, Geonosis and the Arkanis
  approaches, the Techno Union and Banking Clan holdings scattered along
  the Hydian's southern run. The CIS strategy is written in its geography:
  it doesn't need Coruscant; it needs to make the Rim ungovernable and the
  lanes unprofitable.
- **The Mid Rim and Expansion Region are the seam** — majority `Contested`
  by region default, which is where most sector overrides spend their
  budget marking which way each stretch actually leans at tick zero.
- **The blocs sit out, profitably.** Hutt Space (`Hutt Sovereignty`)
  controls the Triellus — the Rim's back road — and both belligerents need
  it open. Corporate Sector, Hapes, Mandalore start `Neutral`, each for
  reasons the Factions section (Bible 03) will cash out as doctrine.

This is deliberately a *pressured* map, not a balanced one: the Republic has
depth, industry, and interior lines; the CIS has initiative, resources, and
shorter supply runs to most of the contested space. Neither can win fast.
What the simulation does with that is the game.

## 4. The Hyperlane Network

### 4.1 Lane classes (content mapping of Framework §2.1)

| Class | Count | What they are |
|---|---|---|
| **Major** | 12 | The superhighways (atlas `major` flag). Fast, famous, high-capacity; news and prices propagate fastest along them (Framework §2.10). Controlling one end-to-end is a war goal in itself. |
| **Minor** | 110 | The named regional routes (Bothan Run, Gamor Run, Enarc Run…). The connective tissue of regional economies — and the smuggler's and raider's map, since patrol coverage thins here first. |
| **Local** | ~12,100 edges | Unnamed short hops in the routing graph. Slow, obscure, unpatrolled by default. Collectively they are why blockades leak: cutting a superhighway reroutes trade onto locals at brutal time cost rather than stopping it. |

### 4.2 The Twelve — strategic identity of the major lanes

Each superhighway gets one line of strategic identity now (full treatment
belongs to the sections that use them):

| Lane | Runs | Strategic identity at tick zero |
|---|---|---|
| **Corellian Run** | Corellia → Ryloth and beyond | The Republic's southern lifeline; the war's most fought-over artery. |
| **Corellian Trade Spine** | Corellia → the far southwest Rim | Second southern trunk; whoever holds both Corellian routes owns half the Rim's trade. |
| **Hydian Way** | Crosses the whole disk, north to far south | The longest road in the galaxy and the CIS's favorite target — cut it anywhere and the Republic's east bleeds. |
| **Rimma Trade Route** | Southern Core → southwest Rim | Industrial corridor (Sullust, Eriadu); Eriadu's seat makes it politically loud. |
| **Namadii Corridor** | Coruscant → the north | Northern trunk toward Mandalorian space; quiet until Mandalore isn't neutral. |
| **Byss Run** | Into the Deep Core | Short, strange, and Republic-locked; matters to whoever wants what the Deep Core hides. |
| **Triellus Trade Route** | The long way around the Rim | Hutt Space's toll road; the war's black-market bloodstream. |
| **Ison Corridor** | Sparse northwest spur | A poor cousin lane — which is exactly why quiet cargo prefers it. |
| **Salin Corridor** | Rim connector, southeast | CIS-leaning approach lane into contested space. |
| **Llanic Spice Run** | Southwest Rim diagonal | Spice and gunrunning by design; patrol coverage near zero. |
| **Nothoiin Corridor** | Northeast reach | Frontier connector feeding the Tion approaches. |
| **Hollastin Run** | Hutt Space feeder | Where Hutt commerce meets everyone else's war. |

### 4.3 Chokepoints

A chokepoint is any system whose loss forces multi-day reroutes onto Local
edges — the router (Framework §2.1) makes them *discoverable rather than
declared*: they are simply where betweenness is high and alternatives are
slow. The Bible's job is only to guarantee the famous ones exist as authored
systems in Phase 2 content: **Corellia** (two superhighways interlock),
**Eriadu** (Rimma/Hydian junction country), **Malastare** (mid-Hydian), and
the **Triellus/Hollastin meeting** in Hutt Space. Faction AI will find the
rest on its own — and so will pirates.

## 5. What This Section Feeds

- **Bible 02 (Timeline & Era Rules):** the pressured map above is the
  board state; era rules define the standing forces acting on it.
- **Bible 03 (Factions):** every faction's doctrine gets written against
  this geography (the CIS *is* its lane-end holdings).
- **Framework §2.1/§2.3:** region defaults, sector overrides, and system
  exceptions become the political seed data of the Phase 2 import; the
  Twelve get their `Lane.*` IDs and class flags from the atlas as-is.
- **Phase 2 authored-systems shortlist:** must include the §4.3 chokepoints
  plus at least one world per allegiance state, so every political texture
  is visitable from the first playable galaxy.

## Pillar Check

- **1 — The Galaxy Lives:** everything here is tick-zero state and standing
  pressure; nothing is scheduled. The map is authored to be *unstable in
  interesting directions*, then handed to the sim. *Serves.*
- **2 — Systems Over Scripted Events:** chokepoints are discovered by the
  router, not flagged by fiat; the war plan is implied by geography, not
  written as events. *Serves.*
- **5 — Immersion:** lane-centric politics means what the player sees
  (traffic, prices, patrols, news delay) *is* the political map. *Serves.*
- **8 — Design for Expansion:** Unknown Regions and Wild Space are held
  deliberately empty; three-tier authoring means repainting politics for
  any future era (or mod) is ~100 data entries. *Serves.*

## Scalability Check

- ~100 authored assignments paint 4,143 systems; the number that must be
  hand-maintained does not grow with system count. *Passes.*
- The 736-sector map is used as an *override namespace*, never a work
  list. Any design that requires "for each sector, decide…" is rejected
  in this Bible on sight. *Standing rule.*
- The 157 `Other`-region systems are a bounded, tracked data debt.
  *Watched.*

---

*Changelog*
- 2026-07-02 — v1. Regions, tick-zero political map, lane structure.
