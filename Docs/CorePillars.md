# Core Pillars

Pillars exist to make decisions for us when we're tired, excited, or tempted.
Every design or technical argument should end with "which pillar wins?"

They are **ranked**. When two pillars conflict, the lower number wins.

---

## 1. The Galaxy Lives Without the Player

The simulation is the product. Factions, economies, and NPCs advance on their
own clock.

- **In practice:** every gameplay system must answer *"what does this do when
  the player isn't there?"* before it answers *"what does the player press?"*
- **Rules out:** systems that only exist relative to the player (enemies that
  spawn because the player approached, prices that only move when the player
  trades).
- **Test:** run the sim headless for an in-game week; the state diff must be
  meaningful and explainable.

## 2. Systems Over Scripted Events

Interesting moments emerge from systems interacting, not from triggers.

- **In practice:** authored content (a named character, a scripted battle) is
  allowed only as *seed data* the simulation takes over.
- **Rules out:** quest scripting frameworks, cutscene-driven story beats,
  one-off event code paths.

## 3. Player Freedom Above Linear Progression

Any role, any time, reversibly. Progression unlocks *capability*, never
*permission to continue*.

- **In practice:** careers are emergent labels for what you're doing, not
  selected classes. A trader who picks up a blaster is now also a fighter.
- **Rules out:** class locks, mandatory tutorials, level gates on regions.

## 4. Everything Is Modular

Every major system is a self-contained unit with a narrow public interface,
usable and testable in isolation.

- **In practice:** the layering and dependency rules in
  TechnicalArchitecture.md are pillar-enforced, not suggestions. A system that
  reaches across layers gets refactored before it gets features.
- **Rules out:** god classes, systems that require the whole game running to
  test.

## 5. Immersion Over Convenience (When Practical)

Friction that reinforces living-in-the-galaxy is a feature.

- **In practice:** information is diegetic where possible (HoloNet news, market
  boards, rumors) instead of omniscient UI. Travel is real, not a fast-travel
  menu — hyperspace is a system, not a loading screen.
- **The "when practical" clause:** we playtest friction honestly. If it's
  tedium rather than texture, convenience wins. This pillar loses ties against
  pillars 1–4.

## 6. Every Major System Is Reusable

Systems are written against generic contracts so they serve multiple features.

- **In practice:** the market system doesn't know it's trading tibanna gas; the
  faction AI doesn't know it's the Republic. Star Wars specificity lives in
  data, even though our class and content names embrace the IP.

## 7. Code Quality Over Development Speed

We are building a foundation for years, and building a developer at the same
time.

- **In practice:** every system gets a design doc paragraph before code, and
  code review discipline even solo (self-review diffs before commit). We do
  not merge "temporary" hacks — there is no such thing on a multi-year project.
- **The honest caveat:** quality means *appropriate* engineering, not
  *maximum* engineering. Speculative abstraction is a quality failure too.

## 8. Design for Expansion

Every feature ships with a paragraph on how it grows.

- **In practice:** data schemas are versioned from day one. Systems expose
  extension points (events, data-driven registries) rather than assuming their
  first use is their last.
- **Rules out:** hardcoded enums for open sets (factions, commodities, ship
  classes — all data).

---

## Using the Pillars

- Every design doc ends with a **Pillar Check** section: which pillars does
  this serve, which does it strain?
- When a decision violates a pillar knowingly, we write down *why* in the
  relevant doc. Undocumented violations are bugs.
