# Git Workflow

Solo project, professional habits. The workflow is deliberately lightweight —
process that gets skipped is worse than no process.

## Repository Layout

The Unreal project lives at the repo root (`Hyperlane.uproject` beside
`Docs/`). Generated folders (`Binaries/`, `Intermediate/`, `Saved/`,
`DerivedDataCache/`) are ignored; only source, content, config, and docs are
tracked.

## Git LFS (required, non-negotiable)

Unreal content (`.uasset`, `.umap`) is binary. Without LFS the repo balloons
and every clone downloads every historical version of every asset.
`.gitattributes` routes all binary types through LFS — **verify LFS is
installed (`git lfs install`) before the first content commit.**

Binary assets **cannot be merged**. If the same `.uasset` changes on two
branches, one side loses. Solo mitigation: never edit the same asset on two
live branches; keep branches short-lived.

## Branching Model — trunk-based

- `main` is the trunk. It **always compiles and always opens in the editor**.
  This invariant is the whole model.
- Work happens on short-lived branches: `feature/ship-movement`,
  `fix/docking-crash`, `docs/phase-1-retro`. Days, not weeks; merge or delete.
- Tiny changes (docs, tuning, one-line fixes) may go straight to `main`.
  Judgment, not ceremony.
- No long-lived development branch. Solo, it's pure overhead.
- Merge with `--no-ff` for features (keeps a visible unit of work in
  history); rebase local work freely *before* it's shared with a remote.

## Commits

- Small and coherent: one logical change per commit. "WIP everything" commits
  are memory holes when you return to them in a year.
- Message format:

  ```
  Add docking component to ship pawn

  Ships can now request docking at any actor with a DockingPort.
  Approach validation is distance-only for now; alignment check
  is TODO(phase-1) pending flight-model tuning.
  ```

  Imperative subject ≤ 72 chars; body explains *why* and records known
  limitations. The body is the letter to future-you.
- Never commit a broken build to `main`. Compile before commit — yes, every time.

## Milestones & Safety Net

- Tag every phase completion: `phase-0`, `phase-1`, … with the retrospective
  written first.
- **Push to a private remote (GitHub) from day one.** A multi-year project on
  one laptop's disk is a tragedy scheduled in advance. The remote is the
  backup; push at least at every stopping point.

## Phase-Boundary Ritual

At each phase exit (see Roadmap.md):
1. Sweep `TODO(phase-N)` comments — resolve or re-justify.
2. Write `Docs/Retrospectives/Phase-N.md` — what worked, what didn't, what
   the next phase should change.
3. Tag and push.
