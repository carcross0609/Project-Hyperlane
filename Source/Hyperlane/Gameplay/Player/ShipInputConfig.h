// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ShipInputConfig.generated.h"

class UInputAction;
class UInputMappingContext;

/**
 * The input actions a ship understands, gathered into one asset.
 *
 * *Why an asset rather than hardcoded bindings:* Enhanced Input already
 * keeps *which key* out of code. This keeps *which actions exist* out of
 * the pawn too, so a fighter, a freighter, and a capital ship's helm can
 * carry different control sets without any of them subclassing the pawn to
 * get it. Axis inversion, dead zones, and gamepad-versus-mouse sensitivity
 * are modifiers on the actions themselves — data decisions, not code ones.
 *
 * Nothing here is required. A ship missing an action simply cannot be
 * commanded on that axis, which is a legitimate design state (an automated
 * hauler has no roll control) rather than an error.
 */
UCLASS(BlueprintType)
class HYPERLANE_API UShipInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Added to the local player when this ship is possessed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> MappingContext;

	/** Higher wins when contexts overlap; leave at 0 unless a context must shadow another. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	int32 MappingPriority = 0;

	/** Axis1D. Positive is ahead, negative is reverse. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> ThrottleAction;

	/** Axis2D. X strafes right, Y lifts. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> StrafeAction;

	/** Axis2D. X yaws, Y pitches. Mouse and right stick both feed this one. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> LookAction;

	/** Axis1D. Positive rolls right. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> RollAction;

	/** Digital, held rather than toggled. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> BoostAction;

	/** Digital, one press. Docks when a berth is in reach, undocks when berthed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> DockAction;
};
