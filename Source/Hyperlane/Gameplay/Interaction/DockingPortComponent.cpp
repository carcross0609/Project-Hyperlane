// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#include "Hyperlane/Gameplay/Interaction/DockingPortComponent.h"

#include "Hyperlane/Core/HyperlaneLogging.h"
#include "Hyperlane/Gameplay/Interaction/DockingRegistrySubsystem.h"

UDockingPortComponent::UDockingPortComponent()
{
	// A port is a fixed place that answers queries; it has no per-frame work.
	PrimaryComponentTick.bCanEverTick = false;
}

void UDockingPortComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (UDockingRegistrySubsystem* Registry = World->GetSubsystem<UDockingRegistrySubsystem>())
		{
			Registry->RegisterPort(this);
		}
	}
}

void UDockingPortComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Withdraw before teardown: the registry holds weak pointers, but leaving
	// stale entries makes every later query pay to skip them.
	if (UWorld* World = GetWorld())
	{
		if (UDockingRegistrySubsystem* Registry = World->GetSubsystem<UDockingRegistrySubsystem>())
		{
			Registry->UnregisterPort(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

bool UDockingPortComponent::AcceptsSizeClass(const UShipSizeClassDef* SizeClass) const
{
	if (AcceptedSizeClasses.IsEmpty())
	{
		return true;
	}

	// A ship with no size class is an authoring error upstream. Refusing it
	// here would surface as "docking is broken" rather than "this hull is
	// unconfigured", so let it through and let the ship class warn.
	if (!SizeClass)
	{
		return true;
	}

	return AcceptedSizeClasses.ContainsByPredicate(
		[SizeClass](const TObjectPtr<UShipSizeClassDef>& Accepted)
		{
			return Accepted == SizeClass;
		});
}
