// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#include "Hyperlane/Gameplay/Interaction/DockingComponent.h"

#include "GameFramework/Actor.h"
#include "Hyperlane/Core/HyperlaneLogging.h"
#include "Hyperlane/Gameplay/Interaction/DockingPortComponent.h"
#include "Hyperlane/Gameplay/Interaction/DockingRegistrySubsystem.h"
#include "Hyperlane/Gameplay/Ships/ShipMovementComponent.h"
#include "Hyperlane/Simulation/Data/ShipSizeClassDef.h"

UDockingComponent::UDockingComponent()
{
	// Earns its tick: it polls proximity on an interval and drives the
	// approach transitions frame by frame.
	PrimaryComponentTick.bCanEverTick = true;
}

void UDockingComponent::BeginPlay()
{
	Super::BeginPlay();

	// Sibling lookup rather than an injected pointer: both components live on
	// the same actor, and direct calls within a layer are fine
	// (TechnicalArchitecture §6 — don't event-bus your own leg).
	if (const AActor* Owner = GetOwner())
	{
		Movement = Owner->FindComponentByClass<UShipMovementComponent>();
	}

	// Seed the timer so the first tick scans. Otherwise a ship that spawns or
	// loads a save already beside a berth reports Free for the first interval,
	// which reads as docking being broken rather than merely late.
	TimeSinceScan = PortScanInterval;

	if (!Movement.IsValid())
	{
		UE_LOG(LogShip, Warning,
			TEXT("'%s' has a docking component but no ship movement component; ")
			TEXT("docking will move the ship without ever restoring flight control."),
			*GetNameSafe(GetOwner()));
	}
}

void UDockingComponent::SetSizeClass(const UShipSizeClassDef* InSizeClass)
{
	SizeClass = InSizeClass;
}

void UDockingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const double Dt = static_cast<double>(DeltaTime);

	if (IsTransitioning())
	{
		TickTransition(Dt);
		return;
	}

	// A berthed ship has nothing to scan for; its port is already known and
	// cannot change until it undocks.
	if (DockState == EDockState::Docked)
	{
		return;
	}

	TimeSinceScan += Dt;
	if (TimeSinceScan >= PortScanInterval)
	{
		TimeSinceScan = 0.0;
		ScanForPort();
	}
}

void UDockingComponent::ScanForPort()
{
	const AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		return;
	}

	UDockingRegistrySubsystem* Registry = World->GetSubsystem<UDockingRegistrySubsystem>();
	if (!Registry)
	{
		return;
	}

	UDockingPortComponent* Found =
		Registry->FindBestPortFor(Owner->GetActorLocation(), SizeClass.Get(), Owner);

	TargetPort = Found;
	SetDockState(Found ? EDockState::InRange : EDockState::Free);
}

void UDockingComponent::RequestDockToggle()
{
	switch (DockState)
	{
	case EDockState::InRange:
		BeginDocking();
		break;

	case EDockState::Docked:
		BeginUndocking();
		break;

	default:
		// Free (nothing in reach) and the two transitions are deliberately
		// inert: a mid-approach toggle should not strand the ship halfway.
		break;
	}
}

void UDockingComponent::BeginDocking()
{
	UDockingPortComponent* Port = TargetPort.Get();
	AActor* Owner = GetOwner();
	if (!Port || !Owner)
	{
		return;
	}

	// Claim the berth now, not on arrival: two ships converging on one port
	// would otherwise both believe it free for the whole approach.
	Port->SetOccupyingShip(Owner);

	TransitionStart = Owner->GetActorTransform();
	TransitionTarget = Port->GetComponentTransform();
	TransitionElapsed = 0.0;
	TransitionDuration = Port->DockingDuration;

	SetFlightControlEnabled(false);
	SetDockState(EDockState::Docking);

	UE_LOG(LogShip, Log, TEXT("'%s' beginning approach to '%s'."),
		*GetNameSafe(Owner), *Port->GetReadableName());
}

void UDockingComponent::BeginUndocking()
{
	UDockingPortComponent* Port = TargetPort.Get();
	AActor* Owner = GetOwner();
	if (!Port || !Owner)
	{
		// The station was destroyed while we were berthed. Hand control back
		// rather than leaving the ship frozen forever.
		SetFlightControlEnabled(true);
		SetDockState(EDockState::Free);
		return;
	}

	Owner->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	TransitionStart = Owner->GetActorTransform();

	// Push out along the port's own facing, so a berth set into a hull ejects
	// outward instead of dragging the ship through the station.
	const FVector Clearance = Port->GetForwardVector() * Port->UndockClearance;
	TransitionTarget = FTransform(TransitionStart.GetRotation(),
		TransitionStart.GetLocation() + Clearance,
		TransitionStart.GetScale3D());

	TransitionElapsed = 0.0;
	TransitionDuration = Port->UndockingDuration;

	SetDockState(EDockState::Undocking);

	UE_LOG(LogShip, Log, TEXT("'%s' undocking from '%s'."),
		*GetNameSafe(Owner), *Port->GetReadableName());
}

void UDockingComponent::TickTransition(double DeltaTime)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	TransitionElapsed += DeltaTime;

	const double RawAlpha = (TransitionDuration > UE_KINDA_SMALL_NUMBER)
		? FMath::Clamp(TransitionElapsed / TransitionDuration, 0.0, 1.0)
		: 1.0;

	// Smoothstep rather than linear: a berth approach that starts and stops
	// abruptly reads as a teleport with extra steps.
	const double Alpha = RawAlpha * RawAlpha * (3.0 - 2.0 * RawAlpha);

	Owner->SetActorLocationAndRotation(
		FMath::Lerp(TransitionStart.GetLocation(), TransitionTarget.GetLocation(), Alpha),
		FQuat::Slerp(TransitionStart.GetRotation(), TransitionTarget.GetRotation(), Alpha));

	if (RawAlpha < 1.0)
	{
		return;
	}

	if (DockState == EDockState::Docking)
	{
		// Attach so the ship rides the station. Stations are static in Phase 1,
		// but a berth on a moving capital ship is the same code path.
		if (UDockingPortComponent* Port = TargetPort.Get())
		{
			Owner->AttachToComponent(Port, FAttachmentTransformRules::KeepWorldTransform);
		}

		SetDockState(EDockState::Docked);
		return;
	}

	// Undocking finished: release the berth and hand back the stick.
	if (UDockingPortComponent* Port = TargetPort.Get())
	{
		Port->SetOccupyingShip(nullptr);
	}

	SetFlightControlEnabled(true);
	TargetPort.Reset();
	SetDockState(EDockState::Free);
}

void UDockingComponent::SetFlightControlEnabled(bool bEnabled)
{
	UShipMovementComponent* Move = Movement.Get();
	if (!Move)
	{
		return;
	}

	Move->SetActive(bEnabled);

	// SetActive alone is not enough: the tick function is registered
	// independently, so a deactivated movement component would keep
	// integrating and fight the approach for control of the transform.
	Move->SetComponentTickEnabled(bEnabled);

	if (!bEnabled)
	{
		// Zero on the way in so a returning pilot does not inherit the
		// velocity they arrived with.
		Move->Velocity = FVector::ZeroVector;
		Move->SetThrottleInput(0.0);
		Move->SetStrafeInput(FVector2D::ZeroVector);
		Move->SetRotationInput(0.0, 0.0, 0.0);
		Move->SetBoostActive(false);
	}
}

void UDockingComponent::SetDockState(EDockState NewState)
{
	if (DockState == NewState)
	{
		return;
	}

	DockState = NewState;
	OnDockStateChanged.Broadcast(NewState);
}
