// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#include "Hyperlane/Gameplay/Interaction/StationActor.h"

#include "Components/StaticMeshComponent.h"
#include "Hyperlane/Gameplay/Interaction/DockingPortComponent.h"

AStationActor::AStationActor()
{
	// A station is scenery with a query attached; it has no per-frame work.
	PrimaryActorTick.bCanEverTick = false;

	Hull = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Hull"));
	RootComponent = Hull;

	PrimaryPort = CreateDefaultSubobject<UDockingPortComponent>(TEXT("PrimaryPort"));
	PrimaryPort->SetupAttachment(Hull);

	// Offset clear of the hull's origin so the guided approach ends alongside
	// the station rather than inside it. The value is a placeholder scaled for
	// the Phase 1 test geometry; real stations set this per berth.
	PrimaryPort->SetRelativeLocation(FVector(0.0, 0.0, 3000.0));

	// Facing +Z, i.e. away from the hull, so the undock push ejects outward.
	PrimaryPort->SetRelativeRotation(FRotator(90.0, 0.0, 0.0));
}
