// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Hyperlane/Simulation/Data/ShipFlightModel.h"
#include "ShipMovementComponent.generated.h"

/**
 * Arcade-leaning 6-DOF flight (Roadmap Phase 1).
 *
 * Three translation axes (throttle, strafe, lift) and three rotation axes
 * (pitch, yaw, roll). Both are integrated by *converging on a commanded
 * value* rather than by summing forces: commanded velocity is where the
 * ship is being told to go, and real velocity chases it at a rate the
 * flight model sets. That single choice is what makes the handling
 * arcade-leaning instead of Newtonian — a fighter goes roughly where its
 * nose points, while a capital ship's momentum carries it wide through
 * every turn, with no separate code path for either (Bible 05 §1).
 *
 * The component holds **no tuning constants**. Every number comes from
 * FShipFlightModel, which the pawn resolves from its ship class
 * (Roadmap Phase 1 exit criterion: tuning lives in data, not in code).
 *
 * Commands are plain setters rather than input bindings on purpose: an
 * NPC pilot or an autopilot in a later phase drives this component through
 * exactly the same door the player does (TechnicalArchitecture §7 — if a
 * ship component ever needs to know whether it is the player, that is a
 * design smell).
 */
UCLASS(ClassGroup = (Hyperlane), meta = (BlueprintSpawnableComponent))
class HYPERLANE_API UShipMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

public:
	UShipMovementComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual float GetMaxSpeed() const override;

	// -- Commands

	/** -1 (full reverse) to +1 (full ahead). Values outside are clamped, not rejected. */
	void SetThrottleInput(double InThrottle);

	/** X strafes right, Y lifts up; each -1 to +1. */
	void SetStrafeInput(const FVector2D& InStrafe);

	/** Each -1 to +1, as a fraction of the model's maximum rate for that axis. */
	void SetRotationInput(double InPitch, double InYaw, double InRoll);

	void SetBoostActive(bool bInBoostActive);

	/** Set once the owning pawn has resolved its ship class. */
	void SetFlightModel(const FShipFlightModel& InFlightModel);

	// -- Queries

	const FShipFlightModel& GetFlightModel() const { return FlightModel; }
	double GetSpeed() const { return Velocity.Size(); }
	double GetThrottleInput() const { return ThrottleInput; }
	bool IsBoostActive() const { return bBoostActive; }

	/** Current speed as a fraction of what full throttle would sustain right now. */
	double GetSpeedFraction() const;

private:
	void IntegrateRotation(double DeltaTime);
	void IntegrateVelocity(double DeltaTime);

	/**
	 * Frame-rate independent convergence factor for an exponential approach.
	 * Separated out because both integrators need the identical curve.
	 */
	static double ConvergenceAlpha(double Responsiveness, double DeltaTime);

	/** Defaults are a placeholder; the pawn overwrites this from ship class data. */
	UPROPERTY(EditDefaultsOnly, Category = "Flight")
	FShipFlightModel FlightModel;

	double ThrottleInput = 0.0;
	FVector2D StrafeInput = FVector2D::ZeroVector;
	double PitchInput = 0.0;
	double YawInput = 0.0;
	double RollInput = 0.0;
	bool bBoostActive = false;

	/**
	 * Local-space angular velocity in deg/s, as (Pitch, Yaw, Roll).
	 * Persisting this between frames is what gives rotation momentum — the
	 * ship keeps swinging briefly after the stick centres.
	 */
	FVector AngularVelocity = FVector::ZeroVector;
};
