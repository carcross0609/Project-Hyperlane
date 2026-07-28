// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#include "Hyperlane/Simulation/GalaxySubsystem.h"

#include "Hyperlane/Core/HyperlaneLogging.h"

void UGalaxySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	bSimRunning = true;
	UE_LOG(LogGalaxySim, Log, TEXT("Galaxy simulation initialized at %s (time scale %g)."),
		*Clock.GetCurrentTime().ToString(), Clock.GetTimeScale());
}

void UGalaxySubsystem::Deinitialize()
{
	bSimRunning = false;
	UE_LOG(LogGalaxySim, Log, TEXT("Galaxy simulation shut down at %s."),
		*Clock.GetCurrentTime().ToString());

	Super::Deinitialize();
}

void UGalaxySubsystem::Tick(float DeltaTime)
{
	Clock.Advance(DeltaTime, [this](const FGalacticDateTime& SimTime)
	{
		UE_LOG(LogGalaxySim, Verbose, TEXT("Sim tick: %s"), *SimTime.ToString());
		OnGalacticSimTick.Broadcast(SimTime);
	});
}

ETickableTickType UGalaxySubsystem::GetTickableTickType() const
{
	// The CDO must never tick; real instances decide per-frame via IsTickable.
	return IsTemplate() ? ETickableTickType::Never : ETickableTickType::Conditional;
}

bool UGalaxySubsystem::IsTickable() const
{
	return bSimRunning;
}

TStatId UGalaxySubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UGalaxySubsystem, STATGROUP_Tickables);
}

void UGalaxySubsystem::SetTimeScale(double InTimeScale)
{
	Clock.SetTimeScale(InTimeScale);
}
