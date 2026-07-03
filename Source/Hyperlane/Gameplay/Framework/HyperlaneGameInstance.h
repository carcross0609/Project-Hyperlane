// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "HyperlaneGameInstance.generated.h"

/**
 * Owns lifetime; little logic of its own (TechnicalArchitecture §7).
 * Real work belongs in subsystems it hosts (UGalaxySubsystem), not here —
 * a fat GameInstance is a god class with extra steps.
 */
UCLASS()
class HYPERLANE_API UHyperlaneGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;
};
