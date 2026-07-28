// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#include "Hyperlane/Gameplay/Framework/HyperlaneGameInstance.h"

#include "Hyperlane/Core/HyperlaneLogging.h"

void UHyperlaneGameInstance::Init()
{
	Super::Init();
	UE_LOG(LogHyperlane, Log, TEXT("Hyperlane game instance initialized."));
}

void UHyperlaneGameInstance::Shutdown()
{
	UE_LOG(LogHyperlane, Log, TEXT("Hyperlane game instance shutting down."));
	Super::Shutdown();
}
