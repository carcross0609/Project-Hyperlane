// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HyperlanePlayerController.generated.h"

/**
 * Input routing and possession (TechnicalArchitecture §7). Deliberately
 * empty until Phase 1 brings Enhanced Input and a ship to possess —
 * behavior belongs on pawn components, not accumulated here.
 */
UCLASS()
class HYPERLANE_API AHyperlanePlayerController : public APlayerController
{
	GENERATED_BODY()
};
