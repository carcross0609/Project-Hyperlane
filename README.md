# Project Hyperlane

A single-player living-galaxy sandbox set in the Star Wars Republic era.
The galaxy simulates itself — wars, trade, politics — and the player is free
to be anyone within it. Unreal Engine 5.6, C++.

**Personal, non-commercial learning project.** Uses Star Wars IP; will never
be distributed or sold.

## Read First

| Document | What it answers |
|---|---|
| [Vision](Docs/Vision.md) | What are we building and why? |
| [Core Pillars](Docs/CorePillars.md) | How do we decide when we disagree with ourselves? |
| [Roadmap](Docs/Roadmap.md) | What are we building *now*, and what's deliberately later? |
| [Technical Architecture](Docs/TechnicalArchitecture.md) | The rules of the codebase |
| [Coding Standards](Docs/CodingStandards.md) | How code is written here |
| [Git Workflow](Docs/GitWorkflow.md) | Branching, LFS, commits, milestones |

## Getting Started (macOS)

1. Install **Xcode** (from the App Store), launch it once to accept licenses.
2. Install **Unreal Engine 5.6** via the Epic Games Launcher.
3. Install **Git LFS**: `brew install git-lfs && git lfs install`.
4. Open `Hyperlane.uproject` — the editor prompts to compile on first open.

*(Until Phase 0 completes, the `.uproject` may not exist yet — see
[Roadmap](Docs/Roadmap.md) Phase 0.)*

## Repository Layout

```
Docs/            Design & technical documentation (start here)
Config/          Engine/game .ini configuration        (created with project)
Content/         Unreal assets — all under Content/Hyperlane/
Source/          C++ — layered per TechnicalArchitecture.md §3
Hyperlane.uproject
```

## Project Status

**Phase 0 — Foundation.** Documentation and scaffolding in place; Unreal
project creation is the next step.
