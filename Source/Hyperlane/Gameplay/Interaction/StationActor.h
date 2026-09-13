// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StationActor.generated.h"

class UDockingPortComponent;
class UStaticMeshComponent;

/**
 * A place a ship can dock (TechnicalArchitecture §7).
 *
 * Thin by design: it owns a hull mesh and a berth, and no behaviour. Docking
 * logic lives on UDockingPortComponent so that a berth cut into a capital
 * ship's hangar, a landing pad, or an asteroid outpost reuses it without
 * inheriting from this class.
 *
 * The default berth sits in front of the hull rather than at its centre,
 * because a ship guided to the actor's origin would arrive inside the
 * geometry. Blueprint subclasses move it, or add more ports for more berths —
 * capacity is authored by adding components, not by editing a count.
 *
 * No mesh is assigned here. Which hull represents a station is presentation,
 * bound on an instance or a Blueprint subclass (TechnicalArchitecture §9).
 */
UCLASS()
class HYPERLANE_API AStationActor : public AActor
{
	GENERATED_BODY()

public:
	AStationActor();

	UStaticMeshComponent* GetHull() const { return Hull; }
	UDockingPortComponent* GetPrimaryPort() const { return PrimaryPort; }

private:
	UPROPERTY(VisibleAnywhere, Category = "Station")
	TObjectPtr<UStaticMeshComponent> Hull;

	UPROPERTY(VisibleAnywhere, Category = "Station")
	TObjectPtr<UDockingPortComponent> PrimaryPort;
};
