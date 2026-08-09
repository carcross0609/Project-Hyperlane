// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#include "Hyperlane/Gameplay/Ships/ShipMovementComponent.h"

UShipMovementComponent::UShipMovementComponent()
{
	// A movement component earns its tick (CodingStandards: tick is opt-in):
	// flight is integrated per rendered frame because it is presentation-rate
	// motion, not simulation. Galaxy-scale work stays on the sim clock.
	PrimaryComponentTick.bCanEverTick = true;
}

void UShipMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ShouldSkipUpdate(DeltaTime) || !UpdatedComponent || !PawnOwner)
	{
		return;
	}

	const double Dt = static_cast<double>(DeltaTime);

	// Rotation first: this frame's thrust should be applied along the heading
	// the pilot just steered to, not the one they started the frame with.
	IntegrateRotation(Dt);
	IntegrateVelocity(Dt);

	const FVector Delta = Velocity * Dt;
	if (!Delta.IsNearlyZero())
	{
		FHitResult Hit(1.0f);
		SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), /*bSweep=*/true, Hit);

		if (Hit.IsValidBlockingHit())
		{
			// Slide rather than stop dead. Scraping a hangar mouth should cost
			// you your line, not your flight; Phase 1 has no damage model to
			// charge for the contact.
			SlideAlongSurface(Delta, 1.0f - Hit.Time, Hit.Normal, Hit);
		}
	}

	UpdateComponentVelocity();
}

void UShipMovementComponent::IntegrateRotation(double DeltaTime)
{
	const FVector CommandedRate(
		PitchInput * FlightModel.MaxPitchRate,
		YawInput * FlightModel.MaxYawRate,
		RollInput * FlightModel.MaxRollRate);

	AngularVelocity = FMath::Lerp(
		AngularVelocity,
		CommandedRate,
		ConvergenceAlpha(FlightModel.AngularResponsiveness, DeltaTime));

	if (AngularVelocity.IsNearlyZero())
	{
		return;
	}

	const FRotator FrameRotation(
		AngularVelocity.X * DeltaTime,
		AngularVelocity.Y * DeltaTime,
		AngularVelocity.Z * DeltaTime);

	// Composed on the right, so the delta is applied in the ship's own frame.
	// This is what makes roll meaningful: a rolled ship pitches toward its own
	// belly, not toward world down. Composing on the left would silently give
	// us an aircraft-sim heading model and break inverted flight.
	const FQuat NewRotation = UpdatedComponent->GetComponentQuat() * FrameRotation.Quaternion();
	MoveUpdatedComponent(FVector::ZeroVector, NewRotation, /*bSweep=*/false);
}

void UShipMovementComponent::IntegrateVelocity(double DeltaTime)
{
	const double BoostMultiplier = bBoostActive ? FlightModel.BoostSpeedMultiplier : 1.0;

	// Reverse ignores boost and uses its own (much lower) ceiling — backing up
	// fast is never the answer we want a pilot to reach for.
	const double ForwardSpeed = (ThrottleInput >= 0.0)
		? ThrottleInput * FlightModel.MaxForwardSpeed * BoostMultiplier
		: ThrottleInput * FlightModel.MaxReverseSpeed;

	const FVector CommandedLocal(
		ForwardSpeed,
		StrafeInput.X * FlightModel.MaxStrafeSpeed,
		StrafeInput.Y * FlightModel.MaxStrafeSpeed);

	const FVector CommandedWorld = UpdatedComponent->GetComponentQuat().RotateVector(CommandedLocal);

	Velocity = FMath::Lerp(
		Velocity,
		CommandedWorld,
		ConvergenceAlpha(FlightModel.LinearResponsiveness, DeltaTime));
}

double UShipMovementComponent::ConvergenceAlpha(double Responsiveness, double DeltaTime)
{
	// 1 - e^(-k·dt) is the exact solution to "approach the target at rate k".
	// Preferred over a raw (k·dt) lerp because it cannot overshoot on a long
	// frame and it feels identical at 30 and 240 fps. Flight tuning that only
	// holds at one frame rate isn't tuning, it's luck.
	return 1.0 - FMath::Exp(-Responsiveness * DeltaTime);
}

void UShipMovementComponent::SetThrottleInput(double InThrottle)
{
	ThrottleInput = FMath::Clamp(InThrottle, -1.0, 1.0);
}

void UShipMovementComponent::SetStrafeInput(const FVector2D& InStrafe)
{
	StrafeInput.X = FMath::Clamp(InStrafe.X, -1.0, 1.0);
	StrafeInput.Y = FMath::Clamp(InStrafe.Y, -1.0, 1.0);
}

void UShipMovementComponent::SetRotationInput(double InPitch, double InYaw, double InRoll)
{
	PitchInput = FMath::Clamp(InPitch, -1.0, 1.0);
	YawInput = FMath::Clamp(InYaw, -1.0, 1.0);
	RollInput = FMath::Clamp(InRoll, -1.0, 1.0);
}

void UShipMovementComponent::SetBoostActive(bool bInBoostActive)
{
	bBoostActive = bInBoostActive;
}

void UShipMovementComponent::SetFlightModel(const FShipFlightModel& InFlightModel)
{
	FlightModel = InFlightModel;
}

float UShipMovementComponent::GetMaxSpeed() const
{
	return static_cast<float>(FlightModel.MaxForwardSpeed * FlightModel.BoostSpeedMultiplier);
}

double UShipMovementComponent::GetSpeedFraction() const
{
	const double BoostMultiplier = bBoostActive ? FlightModel.BoostSpeedMultiplier : 1.0;
	const double TopSpeed = FlightModel.MaxForwardSpeed * BoostMultiplier;

	return (TopSpeed > UE_KINDA_SMALL_NUMBER) ? (Velocity.Size() / TopSpeed) : 0.0;
}
