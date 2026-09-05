// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#include "Hyperlane/Gameplay/Interaction/DockingRegistrySubsystem.h"

#include "Hyperlane/Core/HyperlaneLogging.h"
#include "Hyperlane/Gameplay/Interaction/DockingPortComponent.h"

void UDockingRegistrySubsystem::RegisterPort(UDockingPortComponent* Port)
{
	if (!Port)
	{
		return;
	}

	Ports.AddUnique(Port);

	UE_LOG(LogShip, Verbose, TEXT("Docking port '%s' registered (%d total)."),
		*Port->GetReadableName(), Ports.Num());
}

void UDockingRegistrySubsystem::UnregisterPort(UDockingPortComponent* Port)
{
	if (!Port)
	{
		return;
	}

	Ports.Remove(Port);

	// Compact stale entries opportunistically. Ports are few and this only
	// runs on teardown, so the scan is cheaper than carrying dead weak
	// pointers through every later query.
	Ports.RemoveAll([](const TWeakObjectPtr<UDockingPortComponent>& Entry)
	{
		return !Entry.IsValid();
	});
}

UDockingPortComponent* UDockingRegistrySubsystem::FindBestPortFor(const FVector& ShipLocation,
	const UShipSizeClassDef* SizeClass,
	const AActor* RequestingShip) const
{
	UDockingPortComponent* Best = nullptr;
	double BestDistanceSq = TNumericLimits<double>::Max();

	for (const TWeakObjectPtr<UDockingPortComponent>& Entry : Ports)
	{
		UDockingPortComponent* Port = Entry.Get();
		if (!Port)
		{
			continue;
		}

		// A berth this ship already holds still counts as its own best match,
		// so a docked ship keeps reporting its port rather than losing it.
		const AActor* Occupant = Port->GetOccupyingShip();
		if (Occupant && Occupant != RequestingShip)
		{
			continue;
		}

		if (!Port->AcceptsSizeClass(SizeClass))
		{
			continue;
		}

		const double DistanceSq = FVector::DistSquared(ShipLocation, Port->GetComponentLocation());
		const double RadiusSq = Port->ApproachRadius * Port->ApproachRadius;
		if (DistanceSq > RadiusSq || DistanceSq >= BestDistanceSq)
		{
			continue;
		}

		BestDistanceSq = DistanceSq;
		Best = Port;
	}

	return Best;
}
