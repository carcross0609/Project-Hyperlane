# Phase 0 Retrospective — Foundation

**Closed:** 2026-07-27

## Deliverables

- [x] Vision.md, CorePillars.md, Roadmap.md
- [x] TechnicalArchitecture.md, CodingStandards.md, GitWorkflow.md
- [x] Repo scaffolding (.gitignore, .gitattributes, .editorconfig, README)
- [x] UE 5.8 C++ project created (`Hyperlane.uproject`), compiles, runs
- [x] Git LFS verified working with a first binary asset
- [x] Dev environment proven: edit C++ → compile → hot reload → see change

Exit criteria met: the project opens and compiles from a machine that had
none of the toolchain installed at the start of Phase 0.

## What happened

Dev machine started with only Xcode and the Epic Games Launcher/UE 5.8
installed — no Homebrew, no Git LFS. Environment setup:

1. Installed Homebrew, then `git-lfs` via `brew install git-lfs`;
   `git lfs install` to wire the repo hooks.
2. First editor open failed with UE's generic
   "Hyperlane could not be compiled. Try rebuilding from source manually."
   dialog, which gives no error detail. Building directly via
   `Engine/Build/BatchFiles/Mac/Build.sh HyperlaneEditor Mac Development
   -project=... -waitmutex` surfaced the real compiler errors.
3. Root cause: every local `#include` in the hand-authored skeleton used a
   path relative to the module root (e.g. `"Core/HyperlaneLogging.h"`).
   `Hyperlane.Target.cs` sets `DefaultBuildSettings = BuildSettingsVersion.Latest`,
   which disables legacy public include paths — UBT's include search root
   becomes `Source/`, not `Source/Hyperlane/`. Fixed by prefixing all local
   includes with the module name (`"Hyperlane/Core/HyperlaneLogging.h"`).
   10 files, 14 include lines, no logic changes.
4. Verified hot reload by editing a file with the editor open and rebuilding
   from the command line: UBT produced a versioned
   `libUnrealEditor-Hyperlane-0001.dylib` instead of overwriting the loaded
   one — the same mechanism Live Coding uses.

## Known limitations / carried debt

- Git identity was never set globally on this machine, so the include-path
  fix commit was authored with a machine-guessed identity
  (`carsoncrossno@Carsons-MacBook-Pro.local`) instead of the real one. Needs
  `git config --global user.name` / `user.email` set, and optionally
  `git commit --amend --reset-author` on that commit.
- `brew`/`git-lfs` were not resolving on PATH until `eval "$(/opt/homebrew/bin/brew
  shellenv)"` was added to `~/.zprofile` — a fresh shell is required after
  any Homebrew install for its binaries to be found.
- Live Coding's editor-side UI (toolbar Compile button / status bar) was not
  located on this Mac install; hot reload was verified via command-line
  rebuild instead of the in-editor button. Worth revisiting in Phase 1 once
  there's enough iteration volume to make the in-editor button matter.
- UE's failure dialog on first-open compile errors is not diagnostic —
  any future compile failure from a fresh clone should go straight to
  `Build.sh <Target> Mac Development -project=... -waitmutex` rather than
  retrying the editor's own "rebuild from source" prompt.

## Re-plan for Phase 1

No changes to the Phase 1 deliverable list in Roadmap.md. Carry forward: set
git identity, and confirm the include-path convention
(`"Hyperlane/<SubFolder>/<File>.h"`) explicitly in CodingStandards.md so it
isn't rediscovered by trial and error for every new file going forward.
