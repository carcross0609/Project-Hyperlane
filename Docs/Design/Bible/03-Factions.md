# Bible 03 — Factions

The actors. Mechanics live in SimulationFramework §2.3–§2.4; the era's
pressures live in Bible 02 §4. This section is the roster: who exists at
tick zero, what each wants, and the numbers that make them differ.

Roster per DR-09/DR-10: **two monolithic majors, two special factions, eight
authored minors** (headroom to twelve). Every entry uses the sheet template
in §1 so future factions (authored or revolt-spawned) have a mold.

---

## 1. The Faction Sheet Template

Every faction is authored as:

| Field | Meaning |
|---|---|
| ID / Tier | `Faction.*`; `Major / Special / Minor` (tier sets AI depth, Framework §2.3) |
| Doctrine | Four weights, 0–10: **Expansionist / Mercantile / Isolationist / Predatory** — the personality vector feeding utility scoring |
| Holdings | Painted via Bible 01 §3.1 tiers; listed here as identity, not inventory |
| Resources @ tick zero | Treasury / materiel / manpower / influence, in relative bands (Rich/Solid/Lean/Poor) — absolute numbers are Phase 4 tuning |
| Goals @ tick zero | Standing strategic goals the AI starts scoring |
| Tracks | Faction-specific scalars beyond the standard set (cohesion, integrity…) |
| Relations | Only the non-obvious entries; default is Neutral |
| Grows by | What this faction's later content looks like |

## 2. The Majors

### 2.1 The Galactic Republic — `Faction.Republic` (Major)

- **Doctrine:** Expansionist 3 / Mercantile 5 / Isolationist 2 / Predatory 1
  — a status-quo power: it fights to *restore*, not to take.
- **Holdings:** the center and the trunks (Bible 01 §3.3): Core, Deep Core,
  Colonies, Inner Rim; superhighway stretches Core-outward.
- **Resources:** Treasury **Rich** / Materiel **Solid** (industry unmatched,
  conversion just starting) / Manpower **Lean** (the GAR is a flow with one
  source — Bible 02 P2) / Influence **Rich but decaying**.
- **Goals @ tick zero:** hold the Twelve end-to-end; protect Kamino; retake
  seceded sectors nearest the Core; keep the Hutts neutral.
- **Tracks:** **Institutional Integrity** (Bible 02 P4). Bands: **Healthy ≥ 70**
  (normal law, reversible policies); **Strained 40–69** (emergency-power
  operations unlock: nationalized shipping, censorship, tribunals — each use
  lowers the ceiling for recovery); **Captured < 40** (the state can be
  inherited: the Sith coup operation becomes *scorable* — never scheduled).
  Starts at 62 — already strained. That's the era.
- **Texture:** war as administration. The Republic's failures are latency
  and process; its strength is depth. Its tragedy is that every efficient
  wartime fix feeds the ratchet.
- **Grows by:** DR-09 split upgrade — sector commands and political blocs as
  sub-factions when the seam is built.

### 2.2 The Confederacy of Independent Systems — `Faction.CIS` (Major)

- **Doctrine:** Expansionist 6 / Mercantile 8 / Isolationist 1 / Predatory 5
  — a revolt run by its creditors.
- **Holdings:** the rim ends of the lanes (Bible 01 §3.3): Foundry worlds,
  Geonosis and the Arkanis approaches, guild holdings down the outer Hydian.
- **Resources:** Treasury **Rich** (guild backing) / Materiel **Rich**
  (distributed foundries) / Manpower **effectively unmetered but low-grade**
  (droids: quantity with a quality discount, Bible 02 P2 mirror) / Influence
  **Solid in the Rim, nil in the Core**.
- **Goals @ tick zero:** cut the Hydian and one Corellian route; make the
  Rim ungovernable; force recognition, not conquest — Coruscant is a
  bargaining chip in their doctrine, not a target.
- **Tracks:** **Cohesion** (Bible 02 P7) — *this resolves OQ-5.* Bands:
  **Unified ≥ 70** (full operation set); **Strained 40–69** (negotiated-peace
  operations become scorable; costliest offensives get a cohesion surcharge);
  **Fracturing < 40** (v1: separate-peace and defection events fire per
  guild-aligned holding block; post-split-upgrade: sub-factions actually
  instantiate). Moves with war profitability: victories that open markets
  raise it, expensive stalemates burn it. Starts at 78 — flush with
  secession fervor and fresh credit.
- **Texture:** fast, cynical, over-leveraged. Wins look like margin calls.
- **Grows by:** the split upgrade *is* this faction's roadmap — Trade
  Federation, Techno Union, Banking Clan, Commerce Guild as heirs. Build
  trigger: sim runs where cohesion lives below 40 and outcomes feel flat.

## 3. The Specials

### 3.1 The Jedi Order — `Faction.Jedi` (Special; DR-05)

- **Doctrine:** Expansionist 0 / Mercantile 0 / Isolationist 4 / Predatory 0
  — doctrine weights barely describe it; its AI runs on a custom goal set.
- **Holdings:** none. The Temple is a location, not territory. The Order
  fields **Persons** (generals, investigators, envoys) attached to Republic
  operations as *modifiers and actors*, never fleets of its own.
- **Resources:** Manpower **~10,000 and only going down** (Bible 02 P5);
  Influence **Rich but conditional** — spendable only in ways the Republic's
  integrity track can absorb.
- **Goals @ tick zero:** end the war (it scores *peace outcomes*, uniquely
  among belligerent-side factions); protect (assignments to defense
  operations over offense); **investigate** — the Order is the standing
  consumer of evidence chains (Bible 02 §5) and the most likely
  institutional discoverer of the hidden actor.
- **Tracks:** **Moral Integrity** — assassination sanctioned, civilian costs
  accepted, child recruitment continued under attrition… flavor-visible now
  (news tone, Person behavior), Phase 6 fuel later.
- **Content rule (restated from Bible 02):** Jedi are an *attention economy*.
  A Jedi Person on screen is an event; generators must treat them as rare.

### 3.2 The Sith Conspiracy — `Faction.Sith` (Special, hidden; DR-06/DR-25)

- **Doctrine:** Expansionist 9 / Mercantile 2 / Isolationist 0 / Predatory 9
  — scored over *covert* verbs only.
- **Holdings:** nothing under its flag. **Influence stakes** (Bible 02 §5)
  at tick zero, in bands, not named here: deep in the Republic executive
  (the Chancellery's orbit), the CIS head of state's circle, scattered
  minor stakes (a guild director, a crime lord, an admiral or two).
  Authored count target: **≤ 15 stakes** at tick zero.
- **Resources:** Treasury **Solid** (laundered), everything else **via hosts**.
- **Goals @ tick zero:** prolong (war-ending outcomes are scored *negative*);
  ratchet (drive Republic integrity down through crises its stakes
  manufacture); deplete the Jedi (steer them to attritional fronts);
  ascend (coup operation unlocks only at Republic integrity < 40 with
  sufficient stake coverage — Bible 02 P3's outcome space, earned or not).
- **Track:** **Exposure** (Bible 02 §5) — accumulated, independently-sourced
  evidence. The one number that can kill this faction without a battle.
- **Design note:** two named Sith exist as Persons (see Bible 07); the
  *faction* is the conspiracy, and survives its Persons — succession within
  the Rule of Two is a leadership transition, not faction death.

## 4. The Minors (eight authored @ tick zero)

One paragraph each; full sheets grow in data, not in this doc.

- **The Hutt Cartel — `Faction.Hutts`** (E2/M9/I3/P7). Sovereign third power
  (Bible 01 §3.2); owns the Triellus and the war's black-market bloodstream.
  Goals: neutrality-for-profit, tax both sides, punish incursions. Track:
  *Kajidic Rivalry* — internal clan friction as flavor/stability, the seam
  along which a future Hutt civil conflict could be spawned. Rich in
  everything except legitimacy.
- **Black Sun — `Faction.BlackSun`** (E4/M8/I1/P8). The syndicate: spice,
  slicing, extortion, assassination markets. No territory; *presence* tags
  on worlds and lanes (operates like a legal-economy shadow — same market
  systems, contraband ledger). Goals: grow presence where Rim disorder
  (P6) blooms; corrupt patrol funding; stay beneath both war machines.
- **Corporate Sector Authority — `Faction.CSA`** (E3/M10/I5/P4). The galaxy's
  business-neutral enclave (authored `Neutral`). Sells to both sides at
  list price plus war premium. Goals: enforce its charter space, profiteer,
  admit no ideology. The war's quartermaster of last resort.
- **Hapes Consortium — `Faction.Hapes`** (E1/M4/I10/P2). Sealed matriarchal
  monarchy behind the Transitory Mists. Goals: stay closed. Exists as a
  hard-neutral bloc, a rumor mill, and a later-content vault. Cheapest
  authored faction by design.
- **Council of Neutral Systems — `Faction.Neutrals`** (E0/M4/I8/P0). ~1,500
  worlds' diplomatic bloc under Mandalore's leadership (the New Mandalorian
  government). A *treaty faction*: no fleets to speak of, high influence,
  goals scored on keeping members out of the war. Its failure mode — any
  major violating recognized neutrality (Bible 01 §3.2) — is a designed
  crisis generator.
- **Death Watch — `Faction.DeathWatch`** (E6/M1/I2/P9). Mandalorian
  insurgency in exile; micro-faction (single-goal AI): overthrow the New
  Mandalorians, restore warrior rule. Small, violent, and a natural pawn
  for stakes and sponsors — authored partly as a demonstration that
  *minor ≠ safe to ignore*.
- **The Bounty Hunters' Guild — `Faction.BHG`** (E1/M8/I4/P6). Licensing
  body and reputation ledger for the hunt economy. No territory; its
  "holdings" are its contract board — the institutional backbone of the
  Phase 5 bounty career and the sanctioned interface for *hunting the
  player*.
- **The Corsair Compact — `Faction.Corsairs`** (E5/M6/I0/P9, authored
  original). A pirate confederation coalescing where the Triellus meets
  the war's traffic. Exists so tick zero has one *organized* predator
  (ambient piracy is agent-level, Framework §2.9); doubles as the template
  proof for revolt/pirate faction spawning (DR-10's later supplement).

## 5. Relations Matrix @ tick zero (non-obvious entries only)

- Republic ↔ CIS: **AtWar** (the only tick-zero war).
- Jedi ↔ Republic: **Allied** (structurally embedded; not merged — the
  Order can, in principle, walk).
- Sith ↔ everyone: **Neutral on paper** — a hidden faction has no public
  relations row; its true stance lives in goal scoring.
- Hutts ↔ majors: **Neutral**, both courted; each major scores
  "keep-Hutts-neutral" defensively.
- Death Watch ↔ Neutrals: **Hostile** (the bloc's designed internal threat).
- Corsairs ↔ everyone with cargo: **Predatory-neutral** — hostile to
  shipping, invisible to diplomacy.

## 6. What This Section Feeds

- **Bible 07:** seed Persons attach to these factions (leaders, generals,
  underworld). **Bible 08:** military doctrine per faction rides these
  sheets. **Bible 06:** guild/Hutt/CSA economic behavior is doctrine
  applied to markets.
- **DecisionRegister:** OQ-5 resolved (cohesion bands + split build
  trigger, §2.2). New minors within the DR-10 envelope need only a sheet
  here plus data.

## Pillar Check

- **1/2 — Lives, unscripted:** every faction is goals + resources + tracks;
  no faction has an authored fate. The coup, the fracture, the exposure are
  all *scorable states*. *Serves.*
- **6 — Reusable:** one sheet template serves majors, hidden actors, and a
  pirate compact alike; tier sets cost, not kind. *Serves.*
- **8 — Expansion:** growth paths written per faction; split upgrade,
  kajidic rivalry, and Corsair-as-template are pre-designed seams. *Serves.*

## Scalability Check

- Twelve authored actors total; only two run full strategic AI (majors),
  two run custom-but-narrow sets (specials), eight run reduced loops.
  Within Framework §2.3's tiering budget. *Passes.*
- Sith footprint capped: ≤15 stakes, exposure as one scalar. *Passes.*
- Watch item: relation-matrix growth is quadratic in faction count — the
  default-Neutral rule and sparse authoring keep it sub-linear in practice.
  *Watched.*

---

*Changelog*
- 2026-07-02 — v1. Sheet template; 2 majors, 2 specials, 8 minors;
  cohesion/integrity bands (resolves OQ-5); tick-zero relations.
