# Project Hyperlane — Vision

> *Live inside the Star Wars galaxy. Not a campaign — a place.*

## Elevator Pitch

Project Hyperlane is a single-player living-galaxy sandbox set in the Star Wars
Republic era, built in Unreal Engine 5.8 with C++. The galaxy simulates itself:
wars advance, trade routes shift, factions pursue goals, and NPCs live their
own lives whether or not the player is watching. The player is one being among
trillions — free to become a trader, pilot, soldier, Jedi, Sith, bounty hunter,
politician, or nobody at all.

## The Player Fantasy

The core fantasy is **presence, not protagonism**. You are not the Chosen One.
The Clone Wars do not wait for you. If you spend a year hauling spice between
Corellia and Ryloth, the war ends differently than if you had enlisted — but it
ends either way. The moments that feel best are the ones the simulation
produced, not the ones we scripted:

- Arriving at a system to find its prices crashed because a Separatist
  blockade cut off the hyperlane you normally use.
- Hearing on the HoloNet that a battle you fought in (or ignored) changed a
  sector's allegiance.
- A rival trader you've undercut for months finally hiring a bounty hunter.

## What This Game Is

- A **simulation first**, a game second. Systems produce stories.
- **Role-agnostic**: every career path is a lens on the same living galaxy,
  not a separate campaign.
- **Persistent and consequential**: the galaxy has one continuous state that
  the player permanently affects — and that changes without them.
- **Immersive over convenient**: travel takes time, information is local and
  imperfect, the galaxy doesn't pause menus for you (within reason).

## What This Game Is Not (Non-Goals)

Writing these down is as important as the vision itself:

- **Not multiplayer.** Single-player, designed net-aware (see
  TechnicalArchitecture.md) but with no multiplayer commitment.
- **Not a scripted narrative game.** No main quest line. Authored content is
  flavor and seeding, never the spine.
- **Not photorealistic or content-complete Star Wars.** A handful of
  handcrafted-quality locations beats a hundred empty planets.
- **Not commercial.** This uses Star Wars IP by deliberate choice; it is a
  personal learning project and can never be distributed or sold.
- **Not fast.** This is a multi-year project. The foundation matters more
  than the demo.

## Success Criteria

Because this is a personal project, success is defined honestly:

1. **The developer becomes a substantially better engineer.** Every system is
   an excuse to learn a professional pattern properly.
2. **The galaxy demonstrably lives.** At any milestone, we can let the sim run
   for an hour untouched and describe what changed — and why it's interesting.
3. **The codebase stays welcoming.** After months away, any system can be
   understood from its docs and re-entered in under an hour.
4. **There is always something playable.** No six-month stretches where the
   project only exists as architecture.

## Reference Points (and what we take from each)

- **Mount & Blade** — a war simulation the player joins at any level, from
  soldier to lord.
- **X4: Foundations** — a fully simulated economy where every ware is really
  produced, shipped, and consumed.
- **Kenshi** — role-free sandbox where the world is utterly indifferent to you.
- **Star Wars Galaxies** — breadth of civilian professions; being a nobody in
  Star Wars was the point.
- **Dwarf Fortress** — simulation depth generating emergent narrative.

We take *ideas* from these, not scope. Each shipped with a large team or
decades of iteration.
