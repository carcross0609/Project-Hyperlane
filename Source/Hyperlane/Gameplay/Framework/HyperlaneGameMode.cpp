// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#include "Hyperlane/Gameplay/Framework/HyperlaneGameMode.h"

#include "Hyperlane/Gameplay/Player/HyperlanePlayerController.h"
#include "GameFramework/SpectatorPawn.h"

AHyperlaneGameMode::AHyperlaneGameMode()
{
	PlayerControllerClass = AHyperlanePlayerController::StaticClass();

	// TODO(phase-1): replace with AShipPawn once flight exists. A spectator
	// pawn proves the edit -> compile -> fly-around loop until then.
	DefaultPawnClass = ASpectatorPawn::StaticClass();
}
