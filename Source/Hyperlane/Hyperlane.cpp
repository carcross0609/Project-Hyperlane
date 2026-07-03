// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#include "Modules/ModuleManager.h"

// Single primary game module. The layered folders under this module
// (Core/, Simulation/, Gameplay/, UI/) become physical modules only when
// a split trigger fires — see TechnicalArchitecture.md §3.
IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, Hyperlane, "Hyperlane");
