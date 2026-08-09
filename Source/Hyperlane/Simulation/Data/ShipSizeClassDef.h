// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Hyperlane/Simulation/Data/ShipFlightModel.h"
#include "ShipSizeClassDef.generated.h"

/**
 * One of the six size classes from Bible 05 §1 — Strike, Light, Corvette,
 * Frigate, Cruiser, Battleship — as a data asset rather than an enum.
 *
 * *Why not an enum:* Bible 05 is explicit that nothing in the ship section
 * is an enum, and this is the case that proves the rule. The size class
 * owns the **shape of the handling curve**, so adding a size class (or
 * retuning what "Frigate" feels like across every frigate in the game) has
 * to be an authoring act, not a recompile. An enum would put that shape in
 * code and scatter switch statements behind it.
 *
 * A ship class does not copy these numbers; it references a size class and
 * scales the baseline by its own ratings. That is what makes DR-13
 * (pilotable capitals) cost one curve per class rather than one flight
 * model per hull.
 */
UCLASS(BlueprintType)
class HYPERLANE_API UShipSizeClassDef : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Author-facing name: "Strike", "Light", "Battleship". */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	/**
	 * Hull length band in metres (Bible 05 §1). Documentation for whoever
	 * authors ship classes — nothing reads it at runtime yet.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity", meta = (ClampMin = "0.0", ForceUnits = "m"))
	double MinLengthMeters = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity", meta = (ClampMin = "0.0", ForceUnits = "m"))
	double MaxLengthMeters = 0.0;

	/** The handling curve every ship of this size starts from. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight")
	FShipFlightModel BaseFlightModel;
};
