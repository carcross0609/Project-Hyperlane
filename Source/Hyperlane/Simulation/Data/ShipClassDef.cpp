// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#include "Hyperlane/Simulation/Data/ShipClassDef.h"

#include "Hyperlane/Core/HyperlaneLogging.h"
#include "Hyperlane/Simulation/Data/ShipSizeClassDef.h"

FShipFlightModel UShipClassDef::GetResolvedFlightModel() const
{
	if (!SizeClass)
	{
		UE_LOG(LogShip, Warning,
			TEXT("Ship class '%s' has no size class; falling back to default flight model. ")
			TEXT("Assign a UShipSizeClassDef so this hull flies like its class."),
			*GetName());

		return FShipFlightModel();
	}

	FShipFlightModel Resolved = SizeClass->BaseFlightModel;

	Resolved.MaxForwardSpeed *= SublightRating;
	Resolved.MaxReverseSpeed *= SublightRating;
	Resolved.MaxStrafeSpeed *= SublightRating;

	Resolved.MaxPitchRate *= HandlingRating;
	Resolved.MaxYawRate *= HandlingRating;
	Resolved.MaxRollRate *= HandlingRating;

	// Responsiveness scales with handling too, so a nimble hull does not just
	// turn faster — it also stops drifting sooner, which is what "nimble"
	// actually feels like at the stick.
	Resolved.LinearResponsiveness *= HandlingRating;
	Resolved.AngularResponsiveness *= HandlingRating;

	return Resolved;
}
