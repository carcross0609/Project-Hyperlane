// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HyperlaneGameMode.generated.h"

/**
 * Per-map rules; nearly empty for now (TechnicalArchitecture §7).
 * Wires the default framework classes together — nothing more until a
 * map has rules worth owning.
 */
UCLASS()
class HYPERLANE_API AHyperlaneGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHyperlaneGameMode();
};
