# Bible 05 — Ships

The machines. This section fixes the ship **taxonomy**, the **attribute
schema** (resolving OQ-4), and the tick-zero **roster**. Flight *feel* is
Phase 1 tuning; battle *resolution* is Bible 08; what a ship *is* lives
here. Per the Framework (§2.8, §5): classes are `UShipClassDef` data
assets, individual ships at Abstract LOD are counts against those defs,
and nothing in this section is an enum.

---

## 1. Size Classes

Six size classes. Size class sets the *shape* of a ship's handling curve
(DR-13: capitals are directly pilotable — a Venator must fly like 1,100
meters of government property, so the flight model is parameterized by
class from Phase 1 onward), plus docking rules, crew class, and battle
role defaults.

| Class | Length band | Handling curve shape | Crew class |
|---|---|---|---|
| **Strike** | < 30 m | Agile; full 6-DOF authority | Solo/pair |
| **Light** | 20–60 m | Responsive; the player-scale default | Solo–handful |
| **Corvette** | 100–250 m | Deliberate; turns are decisions | Dozens |
| **Frigate** | 300–600 m | Ponderous; fights by positioning | Hundreds |
| **Cruiser** | 600–1,200 m | Strategic; you steer the battle, not the ship | Thousands |
| **Battleship** | > 1,200 m | Terrain that moves | Thousands+ |

Crew is **abstract in v1** (DR-14): a crew-class requirement and quality
level (feeds readiness, Bible 08), not individuals. Walkable interiors
arrive later, player's own ship(s) first.

## 2. Attribute Schema (resolves OQ-4)

Every `UShipClassDef` carries six attribute groups. Sim systems read only
what they need; nothing reads all of it.

| Group | Fields | Consumed by |
|---|---|---|
| **Identity** | ID, name, manufacturer, size class, role tags (`Ship.Role.Freighter`, `.Screen`, `.Carrier`…), faction availability tags | everything |
| **Performance** | mass class, sublight rating, handling rating (feeds class curve), hyperdrive rating (multiplies lane transit, Framework §2.1) | travel, flight, routing |
| **Protection** | hull, shields, signature (how visible to patrols/pirates) | combat, smuggling |
| **Capacity** | cargo units, hangar complement (strike count), passenger berths, crew class + minimum quality | economy, carriers, careers |
| **Combat** | strength value (abstract-battle contribution), loadout profile (anti-strike / anti-hull / point-defense weighting — the matchup triangle input, Bible 08 §4) | battle resolution |
| **Economy** | price band, upkeep rate, legality tags, availability (which archetypes/regions sell it) | markets, faction procurement, player purchase |

Rule: **abstract combat reads exactly two numbers** (strength, loadout
profile) so 10,000 ships stay cheap; everything richer activates only at
Active LOD. If a design wants abstract combat to read a third field,
Bible 08 has to say why.

## 3. The Tick-Zero Roster (24 classes)

Availability note: *entering service* means the class exists in a faction's
production pipeline at tick zero (Bible 02 era rule 1 — authored designs
moving through real shipyard production; content, never a tech tree).

### Republic
| Class | Size | Role | Notes |
|---|---|---|---|
| V-19 Torrent | Strike | Interceptor | Workhorse at tick zero |
| ARC-170 | Strike | Heavy fighter | Entering service |
| LAAT/i gunship | Strike | Assault/dropship | The invasion-sequence texture ship |
| Consular-class | Corvette | Diplomatic/escort | Prewar red trim; being up-gunned |
| Arquitens-class | Frigate | Patrol/escort | Entering service; the lane-security workhorse-to-be |
| Acclamator-class | Cruiser | Assault transport | What Geonosis was won with |
| Venator-class | Battleship | Carrier-battleship | Entering service; the war's icon-in-waiting |

### CIS
| Class | Size | Role | Notes |
|---|---|---|---|
| Vulture droid | Strike | Swarm fighter | No pilot, no fear, no fuel margin |
| Hyena bomber | Strike | Bomber | |
| Droch-class boarding craft | Strike | Boarding | Piracy-adjacent; loved by Corsair salvagers |
| C-9979 | Light | Landing barge | The invasion tell — spotting these *is* intel |
| Diamond-class | Corvette | Screen | Guild security heritage |
| Munificent-class | Frigate | Comms/gun frigate | Banking Clan collateral, repurposed |
| Recusant-class | Cruiser | Destroyer | |
| Lucrehulk-class | Battleship | Carrier-fortress | Converted freight rings; the CIS in one hull |
| Providence-class | Battleship | Command carrier | Entering service |

### Civilian, neutral, and underworld
| Class | Size | Role | Notes |
|---|---|---|---|
| Z-95 Headhunter | Strike | Militia fighter | Everywhere; planetary defense default |
| Ghtroc 720 | Light | Freighter | **Recommended starter ship** — capable, unglamorous, nobody's icon |
| YT-1300 | Light | Freighter | The dream upgrade; deliberately *not* the starter |
| HWK-290 | Light | Courier | Fast, small hold — courier/smuggler split |
| Firespray-31 | Light | Patrol/pursuit | Hunter's ship; BHG cachet |
| Action VI | Frigate* | Bulk freighter | Unarmed tonnage; the convoy system's reason to exist |
| Marauder-class | Corvette | Corsair/CSA patrol | CSA product; Corsair Compact favorite |
| CR70 | Corvette | Multi-role | Blockade runner blank; every minor faction's capital ship |

*Size class by hull length; role tags, not size, carry meaning — an Action
VI is frigate-*sized*, not frigate-*armed*.

Roster discipline: 24 classes is the tick-zero cap. New entries need a
consumer (a faction doctrine, a career, an archetype that sells them) —
ships without users are art debt.

## 4. Player Notes

- **Starter (DR-16):** one battered Ghtroc 720 — decades old, mortgaged
  feel, room to sleep in the hold. The YT-1300 sits one successful year
  away, on purpose.
- **Pilotable capitals (DR-13):** taking the helm of a Venator is legal at
  the sim level from the moment you command one (Phase 6+ fleet career) —
  the class curve, not a permission flag, is what makes it feel enormous.
- **Ownership scales by ledger, not level:** Light is bought, Corvette is
  earned (capital + license), Frigate+ is *commanded* via the military
  path or owned via the industrialist path (DR-11/DR-15).

## 5. What This Section Feeds

- **Bible 06:** freighter capacities and upkeep set trade margins' shape;
  availability tags put hulls in markets. **Bible 08:** strength/loadout
  fields are its resolution inputs; rosters set each faction's composition
  identity. **Phase 1:** the Ghtroc 720 + class-curve requirement is the
  flight-model spec's first line.
- **Register:** OQ-4 resolved (schema above) → DR-28.

## Pillar Check

- **4/6 — Modular, reusable:** one schema serves warships, freighters, and
  the starter junker; abstract combat reads two fields. *Serves.*
- **8 — Expansion:** roster is data with a discipline rule; new factions
  bring sheets + hulls without schema change. *Serves.*

## Scalability Check

- 24 authored classes; abstract fleets are integer counts against them.
  10,000-ship wars cost arrays, not objects. *Passes.*
- Class-parameterized handling means DR-13 costs one curve per size class,
  not one flight model per hull. *Passes.*
- Watch: hangar complements make carriers *two-number* ships whose real
  strength hides in strike counts — Bible 08 must resolve carriers without
  simulating individual fighters at Abstract. *Watched.*

---

*Changelog*
- 2026-07-03 — v1. Size classes, attribute schema (resolves OQ-4),
  24-class roster, starter ship recommendation.
