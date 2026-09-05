// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#include "Hyperlane/Gameplay/Ships/ShipPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Hyperlane/Core/HyperlaneLogging.h"
#include "Hyperlane/Gameplay/Interaction/DockingComponent.h"
#include "Hyperlane/Gameplay/Player/ShipInputConfig.h"
#include "Hyperlane/Gameplay/Ships/ShipMovementComponent.h"
#include "Hyperlane/Simulation/Data/ShipClassDef.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"

AShipPawn::AShipPawn()
{
	// The pawn itself has nothing to do per frame; flight is integrated on the
	// movement component's tick (CodingStandards: tick is opt-in).
	PrimaryActorTick.bCanEverTick = false;

	CollisionRoot = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionRoot"));
	CollisionRoot->InitSphereRadius(400.0f);
	CollisionRoot->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	RootComponent = CollisionRoot;

	Hull = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Hull"));
	Hull->SetupAttachment(CollisionRoot);
	// Art never collides: swapping a Ghtroc for a Venator must not silently
	// change the ship's collision behaviour.
	Hull->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(CollisionRoot);
	CameraBoom->TargetArmLength = 2500.0f;
	// Open space has nothing for the boom to probe against, and the probe
	// would yank the camera through the hull on near misses.
	CameraBoom->bDoCollisionTest = false;
	// The ship's own attitude is the camera's frame — roll must be visible,
	// which pawn control rotation would flatten away.
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 10.0f;

	ChaseCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCamera"));
	ChaseCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	// TODO(phase-2): cockpit/bridge view per DR-14 — a boom length of zero
	// plus a camera socket on the hull, selected by the same view control.

	ShipMovement = CreateDefaultSubobject<UShipMovementComponent>(TEXT("ShipMovement"));
	ShipMovement->UpdatedComponent = CollisionRoot;

	Docking = CreateDefaultSubobject<UDockingComponent>(TEXT("Docking"));
}

void AShipPawn::BeginPlay()
{
	Super::BeginPlay();

	ApplyShipClass();
}

void AShipPawn::ApplyShipClass()
{
	if (!ShipClass)
	{
		UE_LOG(LogShip, Warning,
			TEXT("'%s' has no ship class assigned; flying on placeholder flight defaults."),
			*GetName());
		return;
	}

	ShipMovement->SetFlightModel(ShipClass->GetResolvedFlightModel());

	// Berth eligibility is a size-class question (Bible 05 §1), so the docking
	// component is told the hull's size rather than being handed the whole
	// ship class it would only reach through.
	Docking->SetSizeClass(ShipClass->SizeClass);

	UE_LOG(LogShip, Log, TEXT("'%s' resolved its flight model from ship class '%s'."),
		*GetName(), *ShipClass->GetName());
}

void AShipPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (!InputConfig)
	{
		UE_LOG(LogShip, Warning, TEXT("'%s' has no input config; ship cannot be piloted."), *GetName());
		return;
	}

	// Enhanced Input is the project's only input path; a legacy UInputComponent
	// here means the project settings regressed, not that we should fall back.
	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInput)
	{
		UE_LOG(LogShip, Error,
			TEXT("'%s' got a non-Enhanced input component. Check that the default input classes ")
			TEXT("in Project Settings still point at Enhanced Input."),
			*GetName());
		return;
	}

	if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (InputConfig->MappingContext)
			{
				Subsystem->AddMappingContext(InputConfig->MappingContext, InputConfig->MappingPriority);
			}
			else
			{
				UE_LOG(LogShip, Warning,
					TEXT("Input config '%s' has no mapping context; no key will reach this ship."),
					*InputConfig->GetName());
			}
		}
	}

	// Every axis binds Triggered *and* Completed to the same handler. Completed
	// fires once with a zero value as the key releases; without it the last
	// non-zero command stays latched and the ship never stops turning.
	if (InputConfig->ThrottleAction)
	{
		EnhancedInput->BindAction(InputConfig->ThrottleAction, ETriggerEvent::Triggered, this, &AShipPawn::HandleThrottle);
		EnhancedInput->BindAction(InputConfig->ThrottleAction, ETriggerEvent::Completed, this, &AShipPawn::HandleThrottle);
	}

	if (InputConfig->StrafeAction)
	{
		EnhancedInput->BindAction(InputConfig->StrafeAction, ETriggerEvent::Triggered, this, &AShipPawn::HandleStrafe);
		EnhancedInput->BindAction(InputConfig->StrafeAction, ETriggerEvent::Completed, this, &AShipPawn::HandleStrafe);
	}

	if (InputConfig->LookAction)
	{
		EnhancedInput->BindAction(InputConfig->LookAction, ETriggerEvent::Triggered, this, &AShipPawn::HandleLook);
		EnhancedInput->BindAction(InputConfig->LookAction, ETriggerEvent::Completed, this, &AShipPawn::HandleLook);
	}

	if (InputConfig->RollAction)
	{
		EnhancedInput->BindAction(InputConfig->RollAction, ETriggerEvent::Triggered, this, &AShipPawn::HandleRoll);
		EnhancedInput->BindAction(InputConfig->RollAction, ETriggerEvent::Completed, this, &AShipPawn::HandleRoll);
	}

	if (InputConfig->BoostAction)
	{
		EnhancedInput->BindAction(InputConfig->BoostAction, ETriggerEvent::Triggered, this, &AShipPawn::HandleBoost);
		EnhancedInput->BindAction(InputConfig->BoostAction, ETriggerEvent::Completed, this, &AShipPawn::HandleBoost);
	}

	// Started, not Triggered: docking is a single decision per press. Triggered
	// would repeat while the key is held and toggle the ship straight back out
	// of the berth it just entered.
	if (InputConfig->DockAction)
	{
		EnhancedInput->BindAction(InputConfig->DockAction, ETriggerEvent::Started, this, &AShipPawn::HandleDock);
	}
}

void AShipPawn::HandleThrottle(const FInputActionValue& Value)
{
	ShipMovement->SetThrottleInput(Value.Get<float>());
}

void AShipPawn::HandleStrafe(const FInputActionValue& Value)
{
	ShipMovement->SetStrafeInput(FVector2D(Value.Get<FVector2D>()));
}

void AShipPawn::HandleLook(const FInputActionValue& Value)
{
	LookInput = Value.Get<FVector2D>();
	PushRotationInput();
}

void AShipPawn::HandleRoll(const FInputActionValue& Value)
{
	RollInput = Value.Get<float>();
	PushRotationInput();
}

void AShipPawn::PushRotationInput()
{
	// Y pitches, X yaws. Inversion belongs on the input action as a Negate
	// modifier, so "inverted pitch" stays a settings decision, not a code one.
	ShipMovement->SetRotationInput(LookInput.Y, LookInput.X, RollInput);
}

void AShipPawn::HandleBoost(const FInputActionValue& Value)
{
	ShipMovement->SetBoostActive(Value.Get<bool>());
}

void AShipPawn::HandleDock(const FInputActionValue& /*Value*/)
{
	Docking->RequestDockToggle();
}
