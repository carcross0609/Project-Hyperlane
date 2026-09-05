// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "DockingPortComponent.generated.h"

class UShipSizeClassDef;

/**
 * One berth on a station, platform, or capital ship.
 *
 * A USceneComponent because the port *is* a place: its transform is where a
 * docked ship sits and which way it faces, authored by dragging the component
 * rather than by typing offsets.
 *
 * Ports announce themselves to UDockingRegistrySubsystem on BeginPlay and
 * withdraw on EndPlay. Nothing searches the world for them — discovery by
 * GetAllActorsOfClass is banned (TechnicalArchitecture §6), and a registry is
 * cheaper besides: a query costs O(ports), not O(actors in level).
 *
 * A station carries one of these per berth, so capacity is authored by adding
 * components rather than by a count field that has to stay in sync with art.
 */
UCLASS(ClassGroup = (Hyperlane), meta = (BlueprintSpawnableComponent))
class HYPERLANE_API UDockingPortComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UDockingPortComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** How close a ship must come before this port offers clearance. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Docking", meta = (ClampMin = "0.0", ForceUnits = "cm"))
	double ApproachRadius = 12000.0;

	/**
	 * Which hulls fit (Bible 05 §1: size class sets docking rules).
	 * Empty means "anything" — the honest default for a Phase 1 placeholder,
	 * and it keeps small stations from silently rejecting every ship because
	 * nobody filled the list in.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Docking")
	TArray<TObjectPtr<UShipSizeClassDef>> AcceptedSizeClasses;

	/** Seconds for the guided glide from clearance to berth. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Docking", meta = (ClampMin = "0.0", ForceUnits = "s"))
	double DockingDuration = 2.5;

	/** Seconds for the push-off when leaving. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Docking", meta = (ClampMin = "0.0", ForceUnits = "s"))
	double UndockingDuration = 1.5;

	/** How far the ship is pushed clear along the port's facing before control returns. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Docking", meta = (ClampMin = "0.0", ForceUnits = "cm"))
	double UndockClearance = 4000.0;

	/** Author-facing name for HUD prompts and logs, e.g. "Bay 3". */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Docking")
	FText PortName;

	bool AcceptsSizeClass(const UShipSizeClassDef* SizeClass) const;

	bool IsOccupied() const { return OccupyingShip.IsValid(); }

	/** Claimed at the start of the approach, not on arrival, so two ships never race for one berth. */
	void SetOccupyingShip(AActor* Ship) { OccupyingShip = Ship; }

	const AActor* GetOccupyingShip() const { return OccupyingShip.Get(); }

private:
	/** Weak: a docked ship can be destroyed out from under the port. */
	TWeakObjectPtr<AActor> OccupyingShip;
};
