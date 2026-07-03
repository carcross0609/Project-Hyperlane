// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#include "Gameplay/Framework/HyperlaneAssetManager.h"

#include "Engine/Engine.h"

UHyperlaneAssetManager& UHyperlaneAssetManager::Get()
{
	UHyperlaneAssetManager* Manager = Cast<UHyperlaneAssetManager>(GEngine->AssetManager);

	// Impossible by construction when DefaultEngine.ini registers this
	// class; if it doesn't, nothing downstream can work — fail loudly.
	check(Manager);

	return *Manager;
}
