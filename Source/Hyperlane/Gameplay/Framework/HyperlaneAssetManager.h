// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "HyperlaneAssetManager.generated.h"

/**
 * Asset loading policy (TechnicalArchitecture §7). Becomes load-on-demand
 * policy for the galaxy's data assets (UStarSystemDef, UFactionDef, ...)
 * in Phase 2 — the whole galaxy's data must never be resident at once.
 * Registered via AssetManagerClassName in DefaultEngine.ini.
 */
UCLASS()
class HYPERLANE_API UHyperlaneAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	/** Typed accessor; fatal if the config doesn't register this class. */
	static UHyperlaneAssetManager& Get();
};
