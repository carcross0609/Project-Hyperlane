// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#pragma once

#include "Logging/LogMacros.h"

// One log category per system (CodingStandards §Logging). Categories are
// added here when their system first exists — LogShip, LogEconomy, etc.
// arrive with their phases.

// General framework/glue: game instance lifecycle, mode setup, asset manager.
DECLARE_LOG_CATEGORY_EXTERN(LogHyperlane, Log, All);

// The galaxy simulation: clock, subsystem, and (later) economy/faction models.
DECLARE_LOG_CATEGORY_EXTERN(LogGalaxySim, Log, All);

// Ships: flight model resolution, movement, input binding, docking.
DECLARE_LOG_CATEGORY_EXTERN(LogShip, Log, All);
