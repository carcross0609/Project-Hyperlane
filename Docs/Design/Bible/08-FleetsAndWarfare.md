# Bible 08 — Fleets & Warfare

War, as content. The machinery is fixed: fleets, orders, supply, and
multi-tick abstract resolution in SimulationFramework §2.7–§2.8; strategic
ground war permanently abstract per DR-12; the era's military boundary
conditions in Bible 02 (P1 exhaustion, P2 supply lines, unblooded armies).
This section supplies the organization, doctrines, resolution modifiers,
and the war's texture.

---

## 1. Military Organization

Organization is *naming and doctrine defaults* over Framework fleet
entities — no new mechanics.

- **Republic:** **Sector Fleets** (standing, defensive postures, tied to
  holdings) and **Task Forces** (operation-assigned, built per Bible 02
  P2's logistics arithmetic). Ground forces abstract as **Legions** —
  garrison strength and invasion weight (Framework §2.2/§2.7), carried by
  Acclamators, spent by the invasion sequence (§5).
- **CIS:** **Guild Flotillas** — fleets flagged to guild-holding origins
  (Lucrehulk groups, Munificent screens) under unified command *while
  cohesion holds* (Bible 03 §2.2 bands: at `Strained`, flotillas grow
  loss-averse near their guild's assets; at `Fracturing`, they are the
  units that defect). Droid ground forces are cargo until spent —
  armies-as-materiel, which is the CIS in one design note.
- **Jedi attachment:** a Jedi Person attaches to a Republic fleet/operation
  as a modifier + actor (Bible 03 §3.1) — the Republic's scarcest force
  multiplier, allocated by the Jedi faction's own goals, *not* by Republic
  demand. Friction by design.
- **Minors:** single flotillas with home ranges (Corsair Compact), or
  none (BHG, Black Sun — their violence is agent-scale).

## 2. Doctrine Parameters

Per-faction tuning block on fleet AI (data, Phase 4):

| Parameter | Republic | CIS | Corsairs |
|---|---|---|---|
| Posture bias | Defensive-then-methodical | Raid-and-pressure | Predatory-evasive |
| Loss tolerance | Low (clones are a flow, P2) | High (droids are materiel) | Near-zero |
| Supply discipline | High — halts over risks | Medium — improvises, strands | Lives off prey |
| Veterancy gain | High (survivors learn) | Low (memory wipes with the hull) | High |
| Retreat threshold | Early | Late | Instant |

These five numbers make the two war machines *feel* different in the same
resolver — the Republic husbands veterans it can't replace fast; the CIS
trades hulls for tempo and position.

## 3. Readiness & Veterancy

- **Readiness** (Framework §2.8): supply state × crew quality × time since
  refit. Decays on station, collapses when cut off (P2).
- **Veterancy:** per-fleet scalar, earned in resolution ticks survived.
  At tick zero *everyone is green* (Bible 02 §1) — the war's first year is
  clumsy on both sides, and that's a feature: early battles are bloody,
  lopsided outcomes from doctrine mismatches, and the fleets that survive
  become the war's protagonists. Veterancy transfers on demote/merge
  weighted by composition survival.

## 4. Abstract Resolution Modifiers

The resolver (Framework §2.8: multi-tick, strength + modifiers) reads,
in order of weight:

1. **Supply & readiness** — the biggest lever, by design: logistics wins
   wars (Bible 02 P2).
2. **Composition triangle** from loadout profiles (Bible 05 §2):
   **Strike** beats unscreened capitals → **Screens** (corvettes/
   frigates, point-defense) beat strike → **Capitals** beat screens.
   **Carriers** project strike without owning the triangle leg — resolved
   as *strike-count injection*, never per-fighter simulation (closes
   Bible 05's carrier watch item).
3. **Veterancy differential** (§3).
4. **Person modifiers** — an attached general (Jedi or otherwise, Bible
   07): bounded, meaningful, mortal. Persons can *die in resolution*,
   with all the succession consequences Bible 07 §3 defines.
5. **Position:** fortifications (Fortress archetype), being the blockader
   vs. the runner, terrain tags on the edge/system (nebula, asteroid
   shadow — data on the graph, not geometry).
6. **Droid coordination:** CIS fleets take a readiness floor benefit (no
   fatigue, no morale) but a veterancy ceiling — codifying quality-vs-
   quantity from Bible 02 P2.

Escalation: at Active LOD the same battle renders and the player's actions
feed back as a resolution modifier — the abstract resolver remains the
bookkeeper of record (Framework §1.4, §2.8).

## 5. The Invasion Sequence (DR-12)

Taking a world is five abstract stages, each a fleet order, each
generating events, each interruptible — which is where the stories live:

1. **Blockade** — cut the lanes (Framework §2.1); prices and news react
   immediately; garrison supply reserve starts draining.
2. **Suppress** — reduce orbital defense; bombardment is *available* here
   and cheaper than siege — at stability, unrest, and news-tone costs the
   attacker's government type may not want (a policy input, Framework
   §2.4: the Republic pays reputation where the CIS pays cohesion).
3. **Land** — spend invasion weight (legions/droid tonnage) against
   garrison strength × fortification; multi-day abstract resolution.
4. **Occupy** — sovereignty flips; government degrades to
   `MilitaryOccupation` (Framework §2.4) with its stability penalty.
5. **Stabilize** — the long tail: garrison upkeep, unrest suppression,
   supply commitment. Occupations that skip this stage breed the revolts
   that spawn factions. Conquest is easy; *keeping* is the game.

A siege interrupted at stage 2 by a relief fleet, a blockade that starves
a world into revolt before anyone lands, a stage-5 occupation quietly
abandoned when the front moves — all emergent arrangements of these five
orders.

## 6. War Exhaustion (Bible 02 P1, cashed out)

Per-major scalar. **Sources:** fleet losses (weighted by replacement
difficulty — a Venator hurts more than a Vulture swarm), occupation
upkeep, civilian-side strain (blockaded member worlds, bombardment
events). **Effects:** scales treasury strain and unrest pressure;
at high exhaustion, negotiated-peace operations score higher for both
majors' AI (and CIS cohesion drains faster, Bible 03). **Relief:**
victories, secure trade months, peace feelers. Exhaustion is what makes
the war *end somehow* on a years-scale without a timer.

## 7. Privateers, Salvage, and the War's Edges

- **Letters of marque:** a faction license (Framework §2.11 grants) making
  piracy against one flag legal to another — the sanctioned door between
  the combat careers and the war (Phase 5). Revocable, deniable, and a
  diplomatic incident generator.
- **Salvage:** resolution leaves wreck state at the location (Framework
  §2.8) — a lootable, sellable, fought-over resource stream that makes
  battlefields destinations. Hutt and Corsair economies eat well here.
- **Escort demand:** convoy postings rise with lane risk (Framework
  §2.6) — the war continuously generates honest work for armed nobodies,
  which is Phase 5's career soil.

## 8. What This Section Feeds

- **Bible 09:** enlisted→command career milestones ride §1's org names;
  privateering and escort careers ride §7. **Bible 06:** exhaustion and
  invasion stages are demand shocks the economy prices. **Phase 4 design
  doc:** §2–§6 are its requirements list.

## Pillar Check

- **1/2 — Lives, unscripted:** wars are fought by doctrine parameters and
  logistics arithmetic; every named stage is an interruptible order, not
  an event. *Serves.*
- **3 — Freedom:** the war touches every career (salvage, escort, marque,
  smuggling past blockades) without conscripting the player. *Serves.*
- **7 — Appropriate engineering:** six modifiers, five doctrine numbers,
  one triangle — the resolver stays explainable in the inspector.
  *Serves, deliberately austere.*

## Scalability Check

- Resolution reads two ship numbers + six modifiers regardless of fleet
  size. *Passes.*
- Carriers resolved as strike-count injection — no per-fighter entities at
  Abstract. *Passes (closes Bible 05 watch item).*
- Watch: invasion stage 5 creates long-lived upkeep entities (occupations);
  cap concurrent occupations per major via AI scoring so the galaxy
  doesn't fill with forgotten garrisons. *Watched.*

---

*Changelog*
- 2026-07-03 — v1. Organization, doctrine parameters, veterancy,
  resolution modifiers, invasion sequence, exhaustion, war's edges.
