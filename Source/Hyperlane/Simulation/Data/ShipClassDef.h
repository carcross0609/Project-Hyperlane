// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Hyperlane/Simulation/Data/ShipFlightModel.h"
#include "ShipClassDef.generated.h"

class UShipSizeClassDef;

/**
 * A ship *class* — the Ghtroc 720, the Venator — as a data asset
 * (TechnicalArchitecture §5, Bible 05 §2). Individual ships at Abstract
 * LOD are counts against one of these, never objects, which is what keeps
 * 10,000-ship wars affordable.
 *
 * Bible 05 §2 fixes six attribute groups. Only **Identity** and
 * **Performance** exist here, because only they have a consumer in Phase 1
 * (flight). Protection, Capacity, Combat, and Economy arrive with the
 * systems that read them — Bible 05's own "new entries need a consumer"
 * discipline applied to fields as well as hulls. The groups are named in
 * the design doc; adding them later costs one field cluster each and no
 * re-authoring, since no assets exist yet.
 *
 * Layer note: this is Simulation-layer data and stays sim-pure — stats
 * only, no meshes, no Actor types. Which mesh represents a Ghtroc is a
 * presentation decision, bound on a Blueprint subclass of AShipPawn
 * (TechnicalArchitecture §9).
 */
UCLASS(BlueprintType)
class HYPERLANE_API UShipClassDef : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// -- Identity (Bible 05 §2)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText Manufacturer;

	/**
	 * Which handling curve this hull is shaped by. Required — a ship class
	 * without a size class has no flight characteristics to scale.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	TObjectPtr<UShipSizeClassDef> SizeClass;

	// -- Performance (Bible 05 §2)
	//
	// Ratings are multipliers around 1.0, where 1.0 means "exactly the
	// baseline for my size class". A rating, not an absolute speed, is what
	// gets authored: it keeps every Light freighter comparable to every
	// other one, and lets a size class be retuned once for all of them.

	/** Scales the size class's speeds. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	double SublightRating = 1.0;

	/** Scales the size class's rotation rates and responsiveness. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	double HandlingRating = 1.0;

	/**
	 * Multiplies lane transit time (SimulationFramework §2.1). Lower is
	 * faster. No consumer until hyperspace travel lands in Phase 2; it is
	 * here because it is Performance-group data and authors will want to
	 * fill it in while they are filling in the rest.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (ClampMin = "0.1"))
	double HyperdriveRating = 1.0;

	/**
	 * The size class baseline scaled by this hull's ratings — the numbers a
	 * ship actually flies by.
	 *
	 * Returns struct defaults with a warning if SizeClass is unset: bad
	 * authoring should be loud and survivable, not a crash (CodingStandards:
	 * never validate data with check()).
	 */
	FShipFlightModel GetResolvedFlightModel() const;
};
