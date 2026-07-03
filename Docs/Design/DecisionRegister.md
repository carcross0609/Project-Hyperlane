# Design Decision Register

Every project-level *design* decision, with date and rationale, in one place —
so decisions get made once instead of re-litigated by accident months apart.
Bible sections cite register IDs instead of re-arguing them.

Scope: design decisions. *Technical* deferrals (GAS, module split, Mass
Entity…) stay in TechnicalArchitecture §10; this register never duplicates
them. A decision here can be revisited — by editing its row with a new date
and a changelog note, never silently.

---

## Decided

### Foundations (2026-07-02)

| ID | Decision | Choice | Why |
|---|---|---|---|
| DR-01 | Start date | **22 BBY, days after Geonosis** | Galaxy maximally in motion from tick one; fronts fluid; best showcase for a living sim |
| DR-02 | Canon policy | **Initial conditions only** | Canon defines the starting map and pressures; from tick one, history belongs to the simulation. Scripted beats would kill Pillar 1 |
| DR-03 | Continuity source | **Legends-primary blend** | Legends supplies the atlas, routes, and economic/political depth; newer-canon material cherry-picked freely. Non-commercial; purity buys nothing |
| DR-04 | Political map authoring | **Three-tier inheritance** (11 region defaults → ≤60 sector overrides → ≤40 system exceptions) | ~100 entries paint 4,143 systems; authoring cost independent of system count. "For each sector, decide…" designs are rejected on sight (Bible 01) |

### The Force & Canon People (2026-07-02)

| ID | Decision | Choice | Why |
|---|---|---|---|
| DR-05 | Force at launch | **Jedi Order is a real sim faction; Force powers abstract** (e.g. Jedi general = battle-resolution modifier) until Phase 6 | The Clone Wars without Jedi generals produces visibly wrong war stories; full Force mechanics have no consumer before Phase 6 |
| DR-06 | Sith conspiracy | **Hidden-actor faction** with concealed goals (prolong war, erode Senate, position for coup), acting through fronts | The era's engine. Simulated, it can win, lose, or be *exposed* — Order 66-like outcomes become possible states, never scheduled events (consistent with DR-02) |
| DR-07 | Canon characters | **Seed Persons (~20–40), sim-owned from tick one** — they can die, defect, fade | Names anchor the fantasy ("I flew escort for Plo Koon"); the sim writes their fates |
| DR-08 | Player Force-sensitivity | **Latent trait set at creation** (toggle or weighted random), dormant until Phase 6+ content probes it | No systems debt now; Jedi path stays earned, not selected (Pillar 3) |

### Factions & Warfare (2026-07-02)

| ID | Decision | Choice | Why |
|---|---|---|---|
| DR-09 | Republic/CIS structure | **Monolithic majors in v1; coalition split is a designed later upgrade** | Ship the war first. Framework faction tiers already leave the door open; the CIS is *designed* to fracture — later |
| DR-10 | Minor factions at tick zero | **~8–12 authored** (Hutt kajidics, Black Sun, CSA, Hapes, Mandalorians, pirate/regional powers) | Enough texture to matter, few enough to write each well. Template-generated minors reserved for revolt-spawned factions later |
| DR-11 | Player military ceiling | **Full ladder: enlisted → officer → fleet command** (enlist Phase 5, command Phase 6+) | The Mount & Blade fantasy; costs only order-issuing permission on existing fleet verbs |
| DR-12 | Strategic ground war | **Abstract resolution permanently**; personal-scale ground combat (Phase 6) happens *inside* abstract outcomes | A second warfare sim layer has no fantasy payoff that abstract + personal doesn't cover |

### Ships, Economy, Player (2026-07-02)

| ID | Decision | Choice | Why |
|---|---|---|---|
| DR-13 | Capital ships | **Commandable via fleet orders AND directly pilotable at the helm**; flight model scales by class (a Venator flies like a Venator) | Player choice over rec (rec was command-only). Implication: the flight model must be class-parameterized from Phase 1, which the data-driven tuning rule already requires |
| DR-14 | Ship interiors & crew | **Cockpit/bridge view + abstract crew in v1; walkable interiors later, player's own ship(s) only** | Preserves the ship-as-home fantasy where it's felt most; defers the interior-per-class content explosion |
| DR-15 | Player economic empire | **Yes, late-game** — player-owned industry/stations/companies are economy entities whose owner ID is the player | Framework supports it for free (§2.5–2.6); it's a career, not a system |
| DR-16 | Player start | **Fixed humble start** — one battered ship, a Rim world, a few credits | Purest Pillar 3; replayability comes from the galaxy rolling differently, not from menus |
| DR-17 | Player death | **Save/reload default + opt-in ironman**: death continues the *same galaxy* with a successor character | The galaxy's persistence is the product; ironman showcases it without mandating it |

### Presentation Scope (2026-07-02)

| ID | Decision | Choice | Why |
|---|---|---|---|
| DR-18 | Space-to-planet | **Transition sequences** (orbit → cinematic descent → landed scene). Seamless atmospheric entry **explicitly rejected** as the project's biggest scope trap | Matches TechnicalArchitecture D5 (one level per system); reads as travel, not teleport (Pillar 5) |
| DR-19 | On-foot camera (Phase 6+) | **Third-person primary**, first-person optional later | Suits Star Wars body language; cheaper to animate acceptably solo |
| DR-20 | HoloNet presentation | **Text-first diegetic feeds** (terminal/datapad aesthetic); voice/video later if ever | Generated events need generated *wording*; text scales with a living sim, audio pipelines don't |

### Process & Reality (2026-07-02)

| ID | Decision | Choice | Why |
|---|---|---|---|
| DR-21 | Sequencing | **Interleave**: when UE 5.8 unblocks, Phase 0 completion takes priority; Bible sections continue between build sessions | "Always something playable" (Vision success criterion 4) |
| DR-22 | Time budget | **~10–15 hrs/week** | Calibration fact for all scope advice: phases are measured in months; every "eventually" in this register means *phases*, not weeks |
| DR-23 | Art sourcing | **Learn 3D modeling as part of the project** (player choice over placeholder-first rec). *Guardrail:* modeling is a parallel learning track — placeholders remain the in-engine default, and no phase exit criterion may ever depend on custom art | Chosen for the learning mission (Vision success criterion 1). The guardrail keeps a second multi-year curriculum off the critical path |
| DR-24 | This register | **Lives at `Docs/Design/DecisionRegister.md`**; Bible sections cite DR-IDs | Decisions without a register get re-litigated by accident |

---

## Open Questions

Known-open design questions, each with where it gets decided. Nothing here
blocks current work; anything that starts to, gets promoted to a decision.

| ID | Question | Decide in |
|---|---|---|
| OQ-1 | Re-bin the 157 `Other`-region atlas systems | Phase 2 import (tracked in Bible 01) |
| OQ-2 | The Phase 2 authored-systems shortlist (10–15 worlds; must include Bible 01 §4.3 chokepoints + one world per allegiance state) | Bible 02–04 will inform; finalize before Phase 2 content |
| OQ-3 | Commodity roster & production chains v0 | Bible 06 |
| OQ-4 | Ship class roster + attribute schema | Bible 05 |
| OQ-5 | Trigger conditions for the CIS/Republic coalition split upgrade (DR-09) | Bible 03 (design the seam now, build later) |
| OQ-6 | Ironman successor mechanics (who/where the successor is, what carries over) | Bible 09; save design lands Phase 2 |
| OQ-7 | Character skills model (capability modifiers, never gates) | Phase 6 planning (logged in SimulationFramework §2.11) |
| OQ-8 | Sidious-faction concealment mechanics (how a hidden faction acts without the info system leaking it) | Bible 02 §pressures + Bible 03; framework touch expected in §2.10 |

---

## Agenda

The standing work plan, in order. (Roadmap phases govern engine work; this
agenda governs *design* work and the current blockers.)

1. **Unblock UE 5.8** — download still stalled (`~/UE_5.8/.egstore/Pending`).
   The only critical-path item not made of words.
2. **Phase 0 completion** the moment the engine lands (DR-21): compile, first
   editor open, LFS verify, hot-reload loop → merge scaffold branch to
   `main`, tag `phase-0`, retrospective.
3. **Bible sections in order**, each now unblocked by this register:
   - **02 Timeline & Era Rules** — needs DR-01/02/05/06/07; defines the
     standing pressures and OQ-8's fiction side.
   - **03 Factions** — needs DR-05/06/07/09/10; writes doctrine for 2 majors,
     the Jedi Order, the hidden Sith actor, ~8–12 minors; designs the DR-09
     split seam (OQ-5).
   - **04 Planets** — archetype library; feeds OQ-2 shortlist.
   - **05 Ships** — needs DR-13/14; roster + class-parameterized attribute
     schema (OQ-4) that Phase 1's data-driven flight tuning consumes.
   - **06 Economy & Trade** — commodity roster (OQ-3); Phase 3's content.
   - **07 NPC Life** — persons/agents/cohorts content incl. seed Persons list
     (DR-07).
   - **08 Fleets & Warfare** — needs DR-11/12/13; military organization and
     doctrine per faction.
   - **09 Player Paths** — needs DR-08/11/15/16/17; careers incl. ironman
     successor design (OQ-6).
4. **Phase 2 prep** (after Bible 04, before Phase 2 build): authored-systems
   shortlist (OQ-2) + atlas importer design note.
5. **Standing:** re-plan at each phase boundary per Roadmap; register grows
   as decisions happen; 3D-modeling learning track runs parallel, never
   gating (DR-23).

---

*Changelog*
- 2026-07-02 — v1. Initial register: DR-01…DR-24 from the founding design
  interview; OQ-1…OQ-8; agenda established.
