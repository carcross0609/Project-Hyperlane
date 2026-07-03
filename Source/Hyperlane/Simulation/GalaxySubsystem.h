// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "Simulation/GalaxyClock.h"
#include "GalaxySubsystem.generated.h"

/**
 * The single authoritative owner of galaxy state (TechnicalArchitecture §4.1).
 *
 * A GameInstance subsystem: created once at game start, survives level
 * loads — fly from Coruscant to Tatooine and the sim never blinks. It
 * composes focused plain-C++ sub-objects (FGalaxyClock now; FTravelNetwork,
 * FEconomyModel, FFactionModel in later phases) rather than doing work itself.
 *
 * Communication contract (TechnicalArchitecture §6):
 *  - Upward, this subsystem broadcasts events. It never knows who listens.
 *  - Inward, gameplay calls explicit query methods (const) and, later,
 *    validated Try* command methods. Nothing outside the Simulation layer
 *    mutates sim state directly.
 */
UCLASS()
class HYPERLANE_API UGalaxySubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	// -- USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// -- FTickableGameObject
	// The engine's tickable hook only feeds real frame time to the sim
	// clock; all sim work happens in fixed steps inside FGalaxyClock
	// (sim work never runs per rendered frame).
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;

	/**
	 * Fires once per sim tick with the sim time as of that tick.
	 * Simulation -> up communication is events only; subscribers upstairs
	 * (gameplay, UI) bind here.
	 */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnGalacticSimTick, const FGalacticDateTime&);
	FOnGalacticSimTick OnGalacticSimTick;

	// -- Queries (gameplay reads sim state through these, never internals)

	const FGalacticDateTime& GetGalacticTime() const { return Clock.GetCurrentTime(); }

	double GetTimeScale() const { return Clock.GetTimeScale(); }

	// -- Commands

	/** Scale ≥ 0; zero pauses the sim. Debug fast-forward and hyperspace time compression alike. */
	void SetTimeScale(double InTimeScale);

private:
	FGalaxyClock Clock;

	/** Guards ticking: the subsystem only ticks between Initialize and Deinitialize. */
	bool bSimRunning = false;
};
