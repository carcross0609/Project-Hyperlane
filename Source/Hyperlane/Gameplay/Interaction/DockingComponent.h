// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DockingComponent.generated.h"

class UDockingPortComponent;
class UShipMovementComponent;
class UShipSizeClassDef;

/**
 * Where a ship is in the docking sequence.
 *
 * An enum, deliberately: the no-enums rule (TechnicalArchitecture §5) governs
 * *open sets* — factions, hulls, commodities — where content must be able to
 * add members without a recompile. A state machine is a closed set whose
 * members exist only because code handles each one, so an enum is the honest
 * representation and a data asset would be ceremony.
 */
UENUM(BlueprintType)
enum class EDockState : uint8
{
	/** Nothing in reach. */
	Free,

	/** A berth has offered clearance and will accept this hull. */
	InRange,

	/** Guided approach; the pilot has handed over control. */
	Docking,

	/** Berthed. Flight controls are inert. */
	Docked,

	/** Pushing clear; control returns when this finishes. */
	Undocking
};

/**
 * Gives a ship the ability to dock (Roadmap Phase 1).
 *
 * Owns only the sequence, not the policy: it asks
 * UDockingRegistrySubsystem which berth is available and lets the port
 * declare its own approach radius, timings, and accepted hulls. Adding a
 * station with different docking rules is therefore authoring, not code.
 *
 * The public verb is RequestDockToggle(), which is what a key press *or* an
 * NPC decision calls. There is no player-specific path through this class,
 * per TechnicalArchitecture §7.
 */
UCLASS(ClassGroup = (Hyperlane), meta = (BlueprintSpawnableComponent))
class HYPERLANE_API UDockingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDockingComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Dock if a berth is in reach, undock if berthed, ignore otherwise. */
	void RequestDockToggle();

	/** Told by the owning pawn once its ship class resolves; drives berth eligibility. */
	void SetSizeClass(const UShipSizeClassDef* InSizeClass);

	EDockState GetDockState() const { return DockState; }
	const UDockingPortComponent* GetTargetPort() const { return TargetPort.Get(); }
	bool IsDocked() const { return DockState == EDockState::Docked; }

	/** True while the sequence owns the ship's transform and input should be ignored. */
	bool IsTransitioning() const { return DockState == EDockState::Docking || DockState == EDockState::Undocking; }

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnDockStateChanged, EDockState);

	/** Fires on every transition so HUD and audio react without polling. */
	FOnDockStateChanged OnDockStateChanged;

private:
	void SetDockState(EDockState NewState);
	void ScanForPort();
	void BeginDocking();
	void BeginUndocking();
	void TickTransition(double DeltaTime);

	/** Freezes or restores flight control around a berth. */
	void SetFlightControlEnabled(bool bEnabled);

	EDockState DockState = EDockState::Free;

	TWeakObjectPtr<UDockingPortComponent> TargetPort;
	TWeakObjectPtr<UShipMovementComponent> Movement;

	/** Not a UPROPERTY: a size class def is owned by the ship class asset, which outlives us. */
	TWeakObjectPtr<const UShipSizeClassDef> SizeClass;

	/** Where the ship was when the current transition began. */
	FTransform TransitionStart = FTransform::Identity;
	FTransform TransitionTarget = FTransform::Identity;
	double TransitionElapsed = 0.0;
	double TransitionDuration = 0.0;

	/**
	 * Berth proximity changes far slower than a frame. Re-asking every tick
	 * would spend a registry walk per frame per ship to learn nothing.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Docking", meta = (ClampMin = "0.02", ForceUnits = "s"))
	double PortScanInterval = 0.2;

	double TimeSinceScan = 0.0;
};
