# Bible 07 — NPC Life

The people, as content. Mechanics are fixed in SimulationFramework §2.9
(cohorts / agents / Persons, promotion rules, LOD): this section supplies
the **role template library**, the **seed Person roster** (DR-07), and the
texture rules that make a dock feel inhabited by people rather than
spawners.

---

## 1. Role Template Library (agents)

An agent = role template + state. Templates are data; the tick-zero
library is fourteen. Each is listed with its drive (what its utility loop
maximizes) and the systems it reads.

| Role | Drive | Reads |
|---|---|---|
| **Trader** | margin / round-trip time | prices (news-lagged), lane risk |
| **Bulk hauler** | contract value, safety | convoy postings, war demand |
| **Smuggler** | contraband margin | inspection regimes, patrol funding, Black Sun presence |
| **Pirate** | prey wealth ÷ patrol density | traffic levels, security, fence access |
| **Patrol captain** | coverage of funded edges | patrol budgets (Framework §2.4), piracy reports |
| **Bounty hunter** | posted bounties, reputation | BHG board, target sightings (news-lagged) |
| **Mercenary** | contract value, survival odds | faction operations seeking auxiliaries |
| **Recruiter** | quota | war exhaustion, planet unrest (both sides' texture) |
| **Journalist** | story significance | sim events, evidence records (Bible 02 §5 — journalists are civilian investigators) |
| **Investigator** | chain completion | evidence records; Jedi/security/private variants |
| **Broker/fence** | spread, discretion | local stock, contraband flows |
| **Refugee cohort-leader** | safety, passage | war events, lane status (moves cohorts — the war's saddest cargo) |
| **Dockworker/fixer** | wages, favors | local economy — texture agents; interaction surface for rumor and small jobs |
| **Corsair officer** | Compact standing | Corsair faction goals (the *faction-employed* variant of pirate) |

Rules: templates never check "is this the player's target/friend/quest" —
they read sim state only. Regional counts drift with what the economy and
war support (Framework §2.6 pattern generalized).

## 2. Seed Persons (DR-07): the tick-zero roster

Twenty-eight authored Persons. Sheet = faction, role, one-line agenda; the
sim owns their fates from tick one. Grouped:

### Republic & Senate
| Person | Role | Agenda at tick zero |
|---|---|---|
| Palpatine | Supreme Chancellor | *Public:* win the war honorably. *(See also: Sith.)* |
| Bail Organa | Senator (Alderaan) | Preserve the constitution while funding the war |
| Padmé Amidala | Senator (Naboo) | Negotiated peace, over everyone's objections |
| Mon Mothma | Senator (Chandrila), junior | Watch, learn, count votes |
| Wilhuff Tarkin | Republic officer | Efficiency; rules-of-war are for winners |
| Wullf Yularen | Fleet officer | Professionalize a fleet run by monks |
| Lama Su | PM of Kamino | Deliver product; renegotiate at leverage |

### Jedi Order
| Person | Role | Agenda |
|---|---|---|
| Yoda | Grand Master | End it without losing the Order's soul |
| Mace Windu | Master of the Order | Win it before it can't be won |
| Obi-Wan Kenobi | General | Serve; doubt quietly |
| Anakin Skywalker | Knight, young general | Win everything, personally, now |
| Plo Koon | General | The soldiers' Jedi |
| Luminara Unduli | General | Discipline as compassion |
| Quinlan Vos | Shadow/investigator | The Order's edge case — evidence-chain work |

### CIS
| Person | Role | Agenda |
|---|---|---|
| Count Dooku | Head of State | Deliver a broken Republic to his master *(see: Sith)* |
| General Grievous | Supreme Commander | Body count as doctrine; unfamous at tick zero, not for long |
| Asajj Ventress | Enforcer | Prove herself to Dooku |
| Nute Gunray | Trade Federation Viceroy | Profit, revenge on Naboo, survival — in that order |
| Wat Tambor | Techno Union Foreman | Foundry throughput über alles |
| San Hill | Banking Clan Chair | Finance both sides' interest payments |
| Poggle the Lesser | Geonosian Archduke | Keep the war machine ordering from *his* hives |

### The Sith line (hidden faction, Bible 03 §3.2)
Palpatine (Sidious) and Dooku (Tyranus) are the conspiracy's two Persons —
each carries a **public sheet** (above) and a **hidden sheet** (stake
network role). One secret, held by two Persons, discoverable via Bible 02
§5. No third Sith Person at tick zero.

### Underworld & neutral
| Person | Role | Agenda |
|---|---|---|
| Jabba Desilijic Tiure | Kajidic lord (Tatooine reach) | Tax the war's back door |
| Gardulla Besadii | Rival kajidic lord | Take what Jabba taxes |
| Ziro Desilijic | Kajidic schemer (Coruscant) | Be indispensable to bigger criminals |
| "Underlord Vess Kogo" | Black Sun head (authored original) | Meet the demand the war creates |
| Cad Bane | Freelance hunter | Be the most expensive option |
| Aurra Sing | Freelance hunter | Grudges, retainer optional |
| Satine Kryze | Duchess of Mandalore | Keep 1,500 worlds out of it |
| Pre Vizsla | Death Watch leader | Burn her pacifism down *(publicly: loyal governor)* |

Roster discipline: 28 is the cap at tick zero. Every later named Person is
**promoted, not authored** (Framework §2.9) — the roster grows from play,
except era-consistent authored additions logged here with changelog.

## 3. The Attention Economy (content rules)

- **Jedi are events** (Bible 02/03 restated as a quota): generators keep
  Jedi Persons rare per region-month. A Jedi at your dock should be a
  story you tell.
- **Persons cluster where decisions cluster:** capitals, headquarters,
  front commands. The Rim sees Persons through news, then suddenly in
  person, which is how it should feel.
- **Cohorts are visible as texture, not units:** crowd density, refugee
  flows, recruitment lines — read from cohort state, rendered ambiently.
- **Grief and memory:** when a Person dies, their faction's goal weights
  shift (succession, Framework §2.3 memory), news propagates by
  significance, and *agents who served under them* carry a memory flag —
  cheap, and it makes the war's cost personal at dock level.

## 4. Daily Texture (what NPC life hands the player)

The sim → player surface area, all of it read from state:

- **Rumor** (dock gossip): degraded news items (Framework §2.10) delivered
  socially; fixers sell freshness.
- **Small contracts:** haul this, find them, carry a message quietly —
  generated from agent needs (a trader short a ship, a recruiter short a
  quota, a fence short a courier).
- **Sightings:** Person and fleet movements as tradeable information —
  the knowledge ledger (DR-16/§2.11) made social.
- **Trouble:** an interdiction, a press gang, a hunter with your
  description — always traceable to a system that decided it (piracy
  math, war exhaustion, your own bounty).

## 5. What This Section Feeds

- **Bible 08:** officer quality and Person generals as battle modifiers.
  **Bible 09:** every career's counterparties (the BHG board, brokers,
  recruiters) are these agents. **Bible 06:** traders/haulers/smugglers
  are the economy's moving parts, already specced in Framework §2.6.
- **Phase 4+ content:** the seed Person roster is the faction AI's face.

## Pillar Check

- **1 — Lives:** agents read sim state, never player state; the roster's
  fates are sim-owned from tick one. *Serves.*
- **2 — Unscripted:** two-sheet Persons make the era's biggest secret a
  *property of two entities*, not a plot; promotion-not-authoring grows
  the cast from play. *Serves.*
- **5 — Immersion:** rumor, sightings, and small contracts keep
  information diegetic and social. *Serves.*

## Scalability Check

- 14 templates × drift counts: agent population scales with economy size,
  not system count. *Passes.*
- 28 Persons with two-sheet cost only for two of them. *Passes.*
- Watch: journalist/investigator agents consume evidence records — cap
  active chains per investigator so the hidden-actor system's cost stays
  bounded (echoes Bible 02's watch item). *Watched.*

---

*Changelog*
- 2026-07-03 — v1. 14 role templates, 28 seed Persons (two-sheet Sith
  design), attention-economy quotas, daily-texture surface.
