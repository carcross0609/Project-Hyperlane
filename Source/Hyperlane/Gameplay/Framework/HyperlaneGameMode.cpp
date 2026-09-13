// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#include "Hyperlane/Gameplay/Framework/HyperlaneGameMode.h"

#include "Hyperlane/Gameplay/Player/HyperlanePlayerController.h"
#include "Hyperlane/Gameplay/Ships/ShipPawn.h"
#include "Hyperlane/UI/HyperlaneHUD.h"

AHyperlaneGameMode::AHyperlaneGameMode()
{
	PlayerControllerClass = AHyperlanePlayerController::StaticClass();

	// The C++ base flies but carries no art or bindings. Maps are expected to
	// override this with a Blueprint subclass that supplies a hull mesh, a
	// ship class, and an input config (TechnicalArchitecture §9).
	DefaultPawnClass = AShipPawn::StaticClass();

	HUDClass = AHyperlaneHUD::StaticClass();
}
