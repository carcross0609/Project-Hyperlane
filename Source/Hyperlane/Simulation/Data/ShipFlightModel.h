// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#pragma once

#include "CoreMinimal.h"
#include "ShipFlightModel.generated.h"

/**
 * How a ship handles, as pure numbers (Bible 05 §2, Performance group).
 *
 * This struct is *only* data — no engine types, no Actor, no component. It
 * lives in the Simulation layer so both the sim (which owns ship class
 * definitions) and the Gameplay layer (which flies them) can read it
 * without either owning the other.
 *
 * Nothing here is a magic number in code: a ship's real values come from
 * its size class baseline scaled by its own ratings (UShipClassDef), which
 * is what makes DR-13 affordable — one curve per size class, not one
 * flight model per hull.
 *
 * Units are Unreal's: centimetres per second, degrees per second.
 */
USTRUCT(BlueprintType)
struct FShipFlightModel
{
	GENERATED_BODY()

	// -- Translation

	/** Top sustained speed at full throttle. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Translation", meta = (ClampMin = "0.0", ForceUnits = "cm/s"))
	double MaxForwardSpeed = 30000.0;

	/** Deliberately weak — reverse is a manoeuvre, not a travel mode. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Translation", meta = (ClampMin = "0.0", ForceUnits = "cm/s"))
	double MaxReverseSpeed = 8000.0;

	/** Lateral and vertical translation, used for docking alignment and dogfight jinking. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Translation", meta = (ClampMin = "0.0", ForceUnits = "cm/s"))
	double MaxStrafeSpeed = 10000.0;

	/**
	 * How sharply real velocity converges on commanded velocity, in 1/s.
	 *
	 * This is the arcade knob (Roadmap Phase 1: "arcade-leaning"). High
	 * values make the ship go where it points almost immediately; low
	 * values let momentum carry it wide through a turn. It is also the
	 * single number that separates a Strike fighter from a Battleship
	 * ("terrain that moves", Bible 05 §1).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Translation", meta = (ClampMin = "0.01"))
	double LinearResponsiveness = 2.0;

	// -- Rotation

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation", meta = (ClampMin = "0.0", ForceUnits = "deg/s"))
	double MaxPitchRate = 60.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation", meta = (ClampMin = "0.0", ForceUnits = "deg/s"))
	double MaxYawRate = 45.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation", meta = (ClampMin = "0.0", ForceUnits = "deg/s"))
	double MaxRollRate = 90.0;

	/**
	 * How sharply angular velocity converges on the commanded rate, in 1/s.
	 * Below roughly 8 the ship keeps turning after the stick centres, which
	 * is what makes big hulls feel heavy rather than merely slow.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation", meta = (ClampMin = "0.01"))
	double AngularResponsiveness = 4.0;

	// -- Boost

	/** Multiplies MaxForwardSpeed while boost is held. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boost", meta = (ClampMin = "1.0"))
	double BoostSpeedMultiplier = 1.75;
};
