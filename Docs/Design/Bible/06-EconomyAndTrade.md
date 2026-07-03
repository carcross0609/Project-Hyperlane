# Bible 06 — Economy & Trade

The money, as content. Mechanics are fixed (SimulationFramework §2.5–§2.7:
conserved goods, local prices, daily production, news-lagged information,
traders as agents); the market's design decisions were made in the economy
interview of 2026-07-03 (DR-29…DR-32). This section supplies the commodity
roster, the chains, the currency rule, the tempo targets, and the trade
content the galaxy opens with. Resolves OQ-3.

---

## 1. The Commodity Roster (DR-29: 15 goods, six families)

| Family | Commodity | Produced by (archetype, Bible 04) | The one-line truth |
|---|---|---|---|
| **Raw** | Ore | Mining World | The war's skeleton, dug rimward |
| | Tibanna Gas | Energy World | Blaster breath; Bespin-grade rare |
| | Crystals | Mining World (crystal slots) | Optics, medicine, and prettier things |
| **Refined** | Alloys | Forge World (input: Ore) | Exists for one reason: hulls — and fleet repair |
| | Fuel | Energy World | The lane tax everyone pays |
| **Finished** | Machinery | Forge World (input: Ore) | Everything that makes everything |
| | Arms | Forge World (input: Tibanna) | Demand curve shaped like a war |
| | Droids | Forge World (inputs: Ore, Crystals) | Labor in a box; the CIS's army is this line item weaponized |
| | **Hulls** | Shipyard World (input: **Alloys**) | **The 3-tier showcase** (DR-30): Ore → Alloys → Hulls |
| | Military Supply | Forge/Agri (inputs: Food, Fuel, Arms) | The deliberate aggregate — Framework §2.7's war-logistics good |
| **Life** | Foodstuffs | Agriworld | The Core's quiet dependency |
| | Medicines | Cultural/Agri (input: Crystals) | Priced in desperation |
| **Luxury** | Luxuries | Cultural World | Stores of value with a smile |
| **Contraband** | Spice | Hutt Fief (glitterstim-grade) | Illegal *by policy*, profitable *by policy* |
| | Hot Cargo | nowhere — it's *made* by theft & salvage | Anything whose title died with its owner |

Family is texture; **legality is policy** — contraband status is a per-
polity tag query (Framework §2.4), so Arms are contraband in Council of
Neutral Systems space, Spice is legal tender-adjacent in Hutt space, and
Hot Cargo is whatever the local inspector says it is.

Roster discipline (mirrors Bible 05): 15 is the v0 cap; a new commodity
needs a producer archetype, a consumer, and a story — goods without all
three are spreadsheet rows, not trade.

## 2. Production Chains (DR-30)

- **Depth rule:** every chain is raw → finished (2 tiers), with **one**
  exception — shipbuilding: **Ore → Alloys → Hulls**. Machinery, Arms,
  and Droids deliberately take *raw* inputs so Alloys stay a single-
  purpose strategic good.
- **Why shipbuilding gets the depth:** it's the war's industry. Blockade
  ore worlds → alloy prices spike at forges → hull output slows at Kuat
  and the Foundry worlds → fleets stop being replaced. A three-link chain
  the player can *read on the map* and interdict at any link — strategic
  depth exactly where the era's fantasy wants it, tuning burden nowhere
  else.
- **Military Supply** is the sanctioned aggregate (Food + Fuel + Arms):
  the one good whose recipe spans families, because it *is* the war
  economy in miniature (Bible 02 P1/P2; Bible 08 §4's biggest lever).
- **Hulls** are consumed by faction shipbuilding (fleet replacement,
  Framework §2.7's materiel pipeline) and, later, by the player's
  shipwright purchases — the same stock (DR-15's ownership-is-an-ID).

## 3. Currency (DR-31)

- **One galactic credit.** All prices, all ledgers, one unit. No FX
  subsystem.
- **Acceptance modifiers:** each polity carries a data-driven credit-
  acceptance factor applied at transaction — Hutt and deep-Rim markets
  discount Republic credits (effectively: worse prices for credit-paying
  outsiders), and the factor *drifts with war confidence* (fed by
  Republic institutional integrity and war momentum — Bible 02 P4, Bible
  08 §6). Era-true texture for the cost of one multiplier.
- **Flavor rule, not mechanic:** in low-acceptance space, NPC dialogue and
  contract postings prefer goods ("paid in fuel"), and Luxuries/Spice read
  as stores of value. If a real second currency ever earns its place,
  this is the seam it grows from (logged as the upgrade path; no OQ —
  the modifier design is the decision).

## 4. Price Tempo (DR-32: readable-fast)

Tuning *targets* — constants live in the Phase 3 tuning doc, but these
numbers are the contract:

- **Local shock → visible repricing: 1–2 game days.** A blockade, a
  battle, a pirate surge reprices the affected markets within a play
  session.
- **Regional re-equilibration: 1–2 game weeks.** Traders reroute, spreads
  narrow, the new normal settles.
- **Arbitrage half-life: a few round trips.** A player-found spread pays
  3–5 runs before NPC traders (news-lagged, same information rules)
  crowd it out — the Phase 3 exit criterion ("a designed exploit gets
  arbitraged away") stated as tempo.
- **Baseline drift:** slow seasonal wander so peace is never perfectly
  flat.
- Information lag (Framework §2.10) stays *on top of* all tempos: distant
  prices are days stale by construction, so speed rewards proximity and
  fresh intel, not menu-watching.

## 5. Tick-Zero Trade Patterns (the opening board)

Four standing flows, all derivable from Bible 01/04 geography — authored
here as *content intent*, verified in the Phase 3 inspector:

1. **The Food Run (inward):** Mid Rim agriworlds feed the Core's P9–P10
   maws. The war's first economic casualty when Mid Rim lanes close —
   Coruscant hungry is a *political* event (P4 feeds on it).
2. **The Ore Tide (outward-in):** Rim mining worlds → Forge/Shipyard
   worlds. The CIS sits astride its rim ends by design (Bible 01 §3.3) —
   this flow is what both war machines are actually fighting over.
3. **The War Surge (new at tick zero):** Arms, Fuel, Military Supply
   toward fronts and mobilization centers — the demand spike arriving in
   Bible 02 §1, permanent for the war's duration, migrating with the map.
4. **The Back Road (parallel):** the Hutt Triellus economy — Spice, Hot
   Cargo, embargo-skipping goods, both belligerents' quiet purchases.
   Grows every time the legal economy is interdicted; P6 disorder made
   liquid.

## 6. The Black Market

Not a separate system — the same markets through a different door
(Framework §2.5 + §2.4 contraband law):

- **Fences** (Bible 07 agents) are market access for tagged goods: worse
  spreads, no questions, standing-gated (underworld reputation is the
  license, Bible 09 §2.2).
- **Premium = risk transfer:** contraband margins price inspection
  probability (patrol funding × lane class) — smuggling profit is
  *computed from* the war's patrol map, never set by hand.
- **Hot Cargo intake:** piracy and battlefield salvage (Bible 08 §7) mint
  it; fences launder it back to legal stock at a haircut. The loop that
  makes the Corsair Compact and Hutt fiefs *economies* rather than spawn
  tables.

## 7. Friction & Instruments (v1 scope fence)

- **In v1:** trade licenses where polities require them (license ledger,
  standing-gated); docking/toll fees (Framework §2.1); cargo manifests
  vs. inspection (the smuggling game).
- **Explicitly later** (logged so they don't creep in): cargo insurance,
  futures/contracts-as-instruments, commodity exchanges, player-visible
  loans. Each is a clean extension of existing state; none is Phase 3.

## 8. What This Section Feeds

- **Phase 3 directly:** this + Framework §2.5–2.6 is the economy build
  spec's content half; §4's tempo targets are its tuning contract; the
  inspector must render §5's four flows or the section has failed.
- **Bible 08:** Military Supply and Hulls are the war's two strategic
  goods; §2's chain is the blockade target map. **Bible 09:** every
  economic career's numbers start here.
- **Register:** OQ-3 resolved → DR-29…DR-32.

## Pillar Check

- **1 — Lives:** four standing flows + war surge run with zero player
  input; the player enters an economy already in motion. *Serves.*
- **2 — Unscripted:** smuggling margins, black-market size, and shortage
  events are computed from policy and war state, never authored. *Serves.*
- **5 — Immersion:** stale prices, credit distrust, and paid-in-fuel
  texture make information and politics *felt* at the market screen.
  *Serves.*
- **6 — Reusable:** the market doesn't know tibanna from tea; families
  and legality are tags. *Serves.*

## Scalability Check

- 15 goods × authored markets: every flow inspector-debuggable; roster
  growth is data behind a three-requirements rule. *Passes.*
- One 3-tier chain bounds the tuning surface; Alloys' single purpose
  keeps the chain graph a tree. *Passes.*
- Acceptance modifier: one scalar per polity, no FX state. *Passes.*
- Watch: readable-fast tempo × 4,100 catalog markets — catalog-tier
  systems get *regional* price resolution, not per-market curves
  (authored markets only carry full state), per Framework §1.2. Flag for
  the Phase 3 doc. *Watched.*

---

*Changelog*
- 2026-07-03 — v1. 15-commodity roster, chains (shipbuilding showcase),
  credit + acceptance, tempo targets, tick-zero flows, black market,
  v1 friction fence. Resolves OQ-3 (DR-29…32).
