# Bible 02 — Timeline & Era Rules

The moment, and the rules of the period. This section fixes what is true at
tick zero (DR-01: 22 BBY, days after Geonosis), which truths about the era
must *hold* no matter how far the simulation diverges, and the standing
pressures that make divergence inevitable. Per DR-02, nothing in this
section schedules anything: it loads the spring; the sim releases it.

---

## 1. The Moment: Days After Geonosis

The first shots of the Clone Wars have been fired. What every system of the
simulation sees at tick zero:

- **Politics:** the Confederacy's secession is formalized and armed; the
  Military Creation Act has passed; Chancellor Palpatine holds fresh
  emergency powers, framed as temporary. The map is the pressured board of
  Bible 01 §3.3.
- **Militaries:** the Grand Army of the Republic has just activated — clone
  legions exist, mostly *in transit* from Kamino, doctrine unwritten. The
  CIS droid armies are mobilized and dispersed across guild holdings. Both
  sides are strong on paper and unblooded in practice; fleets start with
  full readiness and shallow veterancy.
- **Economy:** the war-industry demand spike is *just arriving* — durasteel,
  tibanna, fuel, and shipyard capacity reprice over the first simulated
  weeks as faction procurement (Framework §2.7) hits the markets. Civilian
  trade still flows on prewar patterns; militarized lane inspections are
  days old and inconsistent.
- **Information:** Geonosis dominates every HoloNet feed at every propagation
  distance. Fear is galaxy-wide; facts are Core-deep only. Nobody knows how
  long the war will last, and the markets price that uncertainty.
- **The street:** recruitment is surging on both sides, insurance rates on
  Rim shipping have tripled, and every dock in the galaxy is having the
  same argument about whose fault this is.

## 2. Timekeeping

- **Design documents** use BBY/ABY shorthand. **The game does not.** In
  fiction the epoch is the **Declaration of War — Day 0** of the sim; dates
  render as Galactic Standard Calendar offsets ("Year 2, Day 214, three
  years into the war"). A living galaxy whose history diverges (DR-02) has
  no business displaying a calendar counted from an event that may never
  happen.
- **Galactic Standard Calendar** (Legends): 24-hour days, 5-day weeks,
  35-day months, **368-day years**. This is the concrete spec behind the
  Framework's `FGalacticDateTime` and its cadences (hourly/daily/weekly per
  Framework §1.3).
- Local planetary days/years exist as flavor only; the sim runs exclusively
  on Galactic Standard. (Scalability: one calendar; local time is a
  presentation-layer conversion, never sim state.)

## 3. Era Rules

The guardrails: what stays true about the period regardless of how history
diverges. These are *content constraints on authors and generators*, not
runtime scripts — the sim never checks them; the data obeys them.

1. **Technology is frozen at era level.** No invention during the sim.
   New ship classes may *enter service* over the war (authored,
   era-appropriate designs moving through production pipelines — content
   updates, not tech trees). Mature hyperdrive, HoloNet, droids, cloning
   at Kamino scale: yes. Superweapons, cloaking proliferation, galaxy-scale
   shields: no. If it would headline a different era, it doesn't exist.
2. **Clones are Republic-exclusive and single-source.** All clone
   production traces to Kamino. This is an era fact *and* a deliberate
   strategic single point of failure (see Pressure P2).
3. **The Jedi are few.** Order strength ~10,000 at tick zero, replacement
   measured in decades. Jedi capacity only depletes on war timescales
   (Pressure P5). No mass Force awakenings (DR-08 keeps sensitivity rare).
4. **The galaxy is bigger than its militaries.** Neither side can garrison
   even its own claims. Front lines are lanes, not volumes (Bible 01);
   disorder blooms wherever fleets aren't (Pressure P6).
5. **Institutions outlive their health.** The Senate, the courts, the
   guilds, the Jedi Council persist as bodies even while their authority
   decays or is captured — institutions in this era hollow out before they
   fall over (Pressure P4).
6. **Nothing arrives from outside.** No extragalactic invasions, no Unknown
   Regions deus ex machina. The Unknown Regions stay dark (Bible 01 §2) as
   reserved expansion space, not a plot faucet.

## 4. The Standing Pressures

The era's engines. Each pressure is a *disequilibrium loaded at tick zero*,
stated with its systemic representation and the outcome space it opens.
Together they guarantee an interesting history without scheduling one.

### P1 — The War Itself

Two majors with incompatible goals, fresh armies, and economies converting
to war production. **Systemically:** faction goals + war economy (Framework
§2.3, §2.7) plus a *war exhaustion* pressure on both (accumulating from
losses, occupation costs, and civilian strain into faction stability and
treasury). **Outcome space:** military victory either way, negotiated peace,
frozen stalemate, or mutual exhaustion and fragmentation. War exhaustion is
what guarantees the war *ends somehow* on a years-scale rather than
simmering forever — Vision.md's "it ends either way" is carried by this
pressure, not by a timer.

### P2 — The Clone Supply Line

GAR strength is a **flow, not a stock**: attrition is replaced only by
Kamino's output, shipped down real lanes with real transit times. The CIS
mirror is distributed droid foundries — resilient supply, inferior
per-unit quality. **Systemically:** pure Framework §2.5/§2.7 content — a
production chain with one source node. **Outcome space:** Kamino raided,
blockaded, or lost is a *strategic* wound that compounds over months;
protecting it distorts Republic fleet posture forever. One authored fact,
endless emergent consequence — this is the era's premier example of the
whole design philosophy.

### P3 — The Sith Conspiracy (DR-06)

The hidden-actor faction. **Goals:** prolong the war, ratchet the Republic
toward centralized emergency rule, deplete the Jedi, and position for a
coup that inherits whatever survives. Note the asymmetry worth savoring:
*both belligerents are its instruments; it wants neither to win quickly.*
**Systemically:** §5 below. **Outcome space:** coup achieved (an
Empire-analog is born — the sim keeps running under new management);
exposed and purged (the war's logic transforms overnight); decapitated by
events it couldn't control; or outmaneuvered into irrelevance. The player
can be a bystander, an instrument, or the one who pulls the thread.

### P4 — The Centralization Ratchet

Fear moves power to the center. Every war shock (lost battle, terror event,
Core-adjacent raid) makes emergency-power expansion *easier*; every quiet
month lets normalcy claw something back — but the ratchet is asymmetric by
design. **Systemically:** an institutional-integrity track on the Republic
(a faction-level analog of planet stability, Framework §2.4), moved by war
events and P3 manipulation, gating what its government type can do.
**Outcome space:** the Republic that wins (or loses) the war may be a
functioning democracy, a wartime autocracy primed for P3's coup, or
anything between — and *which* is legible to the player through policy
changes, news tone, and dock-level texture (checkpoints, censorship).

### P5 — Jedi Attrition

Ten thousand generals-and-diplomats stretched across a galactic war they
were never built for. **Systemically:** the Jedi faction (DR-05) fields
Persons and abstract battle modifiers; its losses replace on decade
timescales, so its capacity only goes down. Moral strain tracks alongside
numeric strength (an integrity stat: assassinations sanctioned, civilian
costs accepted) — flavor now, Phase 6 fuel later. **Outcome space:** the
Order ends the war diminished-but-intact, gutted, politically captured, or
— if P3 lands its endgame — destroyed. Jedi scarcity is also a *content*
rule: a Jedi Person appearing anywhere is an event, and the sim's economy
of attention should treat it that way.

### P6 — Rim Disorder

The war eats patrol coverage; the Rim notices within weeks. Piracy grows
where wealth flows unprotected (Framework §2.9 pirate agents read exactly
this), Hutt arbitrage and black markets grow around every embargo, and
whole sectors quietly learn to live without whichever government claims
them. **Systemically:** zero new mechanics — this pressure *is* the
security/prosperity/patrol-funding loop already specified, seeded with a
war that guarantees the inputs move. **Outcome space:** the war's shadow
economy can end up bigger than some member economies; postwar, whoever
"won" inherits a Rim that spent years ungoverned — the sequel war's
initial conditions, written by simulation.

### P7 — Corporate Interest

The guilds bankrolling the CIS are in it for margins, not manifestos.
**Systemically:** doctrine weights (mercantile-predatory) on the CIS's
guild-aligned holdings and, at v1's monolithic grain (DR-09), a *cohesion*
track on the CIS moved by war profitability: victories that open markets
raise it; expensive stalemates and trade collapse lower it. **Outcome
space:** this is the designed seam for the DR-09 coalition split (OQ-5) —
when sub-factions arrive, cohesion is already the number they fracture
along. Until then it modulates CIS willingness to negotiate. A war that
stops paying its sponsors starts ending.

## 5. The Hidden Actor (resolving OQ-8)

How a faction acts for years without the information system (Framework
§2.10) leaking it. Three design rules make concealment *systemic* rather
than authored immunity:

1. **No flag, no assets.** The Sith faction owns no territory, no fleets,
   no markets. Its holdings are **influence stakes**: covert control marks
   attached to *other factions' entities* — a senator, an admiral, a guild
   director, a crime lord (Persons and offices, Framework §2.9).
2. **It acts only through its hosts' legal verbs.** The conspiracy never
   issues "Sith commands." It *biases decisions*: a staked entity's
   utility scoring carries a corrupted-advisor weight, so the Republic
   deploys a fleet foolishly, the CIS refuses a good peace, a bill
   advances at the perfect moment. Every action the world observes is an
   ordinary action by an ordinary actor — attribution goes to the host.
   (Cost check: this reuses faction-AI scoring wholesale; concealment adds
   a modifier layer, not a second AI.)
3. **Evidence exists anyway.** Every biased decision emits an **evidence
   record** — attributed to the host, carrying a faint link in a chain
   (payments, meetings, patterns). Evidence is ordinary information-system
   content: it propagates, degrades into rumor, gets buried. Investigators
   — a Jedi faction goal, journalists, rival intelligence, *the player* —
   can accumulate chains; enough accumulated, independently-sourced
   evidence crosses an exposure threshold and the revelation cascades
   (news shock, purge politics, P4 lurch in either direction).

Rule 3 is the honesty clause: the conspiracy is discoverable **in
principle from tick one** (Pillar 2 — no scripted immunity, no scripted
reveal). Its safety is statistical (chains are long, evidence decays,
hosts are expendable), not authored. And it cuts both ways: the Sith
faction can spend actions *burning evidence* — which are themselves
actions that leave evidence.

## 6. History & the Chronicle

Divergent history is worthless if nobody can read it. The **Chronicle** is
the era's memory:

- **The chronicle is a view, not a truth.** It derives entirely from the
  sim event log (Framework §1.5): events above significance thresholds
  (battles over X strength, sovereignty changes, faction leaders dying,
  exposure cascades) are promoted to chronicle entries. No separate
  history state to maintain or desync.
- **In fiction** it surfaces as HoloNet archives, anniversary
  retrospectives, and bar-room "three years ago today" talk — all
  observer-relative (Framework §2.10): the CIS chronicle of a battle is
  not the Republic's, and neither is complete.
- **Per save, by construction:** each playthrough's chronicle is its own
  alternate history — the artifact that proves DR-02 (Vision.md success
  criterion 2: "describe what changed — and why it's interesting").

**Illustrative, non-normative** — one plausible Year-1 chronicle, to fix
intent: *the CIS cuts the mid-Hydian in month 2 (P1); Republic counter-
convoys strip Rim patrols (P6); a pirate confederation seizes a Triellus
junction and the Hutts tax both sides' war anyway; a stalemate at
Malastare bleeds both fleets (war exhaustion ticks up); an emergency
logistics act centralizes shipping under the Chancellor's office (P4,
nudged by P3); a Jedi-led investigation into a leaked deployment closes
with a conveniently dead middleman (evidence chain broken — this time).*
No entry above is authored; every one is reachable by the systems as
specified. That is the bar.

## 7. What This Section Feeds

- **Bible 03 (Factions):** every pressure lands as faction goals, tracks,
  and doctrine — P3's influence stakes and P7's cohesion are faction-sheet
  line items; the Jedi and Sith get full faction treatments there.
- **Bible 06 (Economy):** the tick-zero demand spike and P2's clone
  logistics chain become commodity/production content.
- **Bible 08 (Warfare):** war exhaustion, veterancy-from-unblooded, and
  P2/P6 posture distortions become the military design's boundary
  conditions.
- **Framework touches (flagged, not yet specced):** influence stakes +
  evidence records extend §2.9/§2.10; war exhaustion and
  institutional-integrity/cohesion tracks extend §2.3/§2.4. All four ride
  existing patterns; none demands a new system. To be specced in the
  relevant per-phase design docs (Phase 4).
- **DecisionRegister:** OQ-8 resolved by §5 (register updated this
  commit); OQ-5 sharpened — cohesion is the designated splitting number.

## Pillar Check

- **1 — The Galaxy Lives:** the era is expressed as seven disequilibria
  and zero scheduled events; history is guaranteed to *happen*, not
  guaranteed to happen *any particular way*. *Serves.*
- **2 — Systems Over Scripted Events:** the era's most script-tempting
  element (the Sith plot) is systemized with a discoverability clause —
  no scripted immunity, no scripted reveal. *Serves, deliberately hard.*
- **5 — Immersion:** in-fiction calendar epoch, observer-relative
  chronicle, pressure outcomes legible through diegetic texture. *Serves.*
- **8 — Design for Expansion:** era rules are authoring guardrails, so a
  future era swap (or postwar continuation) is a data/content change; P6
  explicitly writes the sequel's initial conditions. *Serves.*

## Scalability Check

- Seven pressures ride five existing Framework mechanisms; the four
  genuinely new elements (stakes, evidence records, exhaustion,
  integrity/cohesion tracks) are modifier layers and scalar tracks — no
  new heavy systems. *Passes.*
- The hidden actor adds no per-system cost across 4,143 systems: its
  footprint scales with *stakes held* (dozens), not with galaxy size.
  *Passes.*
- Chronicle = thresholded view of an event log that already exists;
  storage bounded by significance threshold. *Passes.*
- Watch item: evidence-record volume if stake count grows — cap active
  chains per stake, decay aggressively. *Watched.*

---

*Changelog*
- 2026-07-02 — v1. The moment, calendar, six era rules, pressures P1–P7,
  hidden-actor design (resolves OQ-8), chronicle design.
