// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ShipPawn.generated.h"

class UCameraComponent;
class UShipClassDef;
class UShipInputConfig;
class UShipMovementComponent;
class USphereComponent;
class USpringArmComponent;
class UStaticMeshComponent;
struct FInputActionValue;

/**
 * A flyable ship: a thin socket for components (TechnicalArchitecture §7).
 *
 * Derives from APawn rather than ACharacter deliberately — Character drags
 * in a humanoid walking-movement component that is pure dead weight on a
 * starship.
 *
 * The pawn owns almost no behaviour. It resolves its flight numbers from
 * its ship class, translates input events into movement commands, and
 * otherwise gets out of the way. Everything a rendered NPC ship would also
 * need lives on the components, so an NPC freighter is this same set of
 * components without the input bindings.
 *
 * What it is *not*: the mesh. Which hull represents this ship is bound on a
 * Blueprint subclass (TechnicalArchitecture §9) so that art never round-
 * trips through C++.
 */
UCLASS()
class HYPERLANE_API AShipPawn : public APawn
{
	GENERATED_BODY()

public:
	AShipPawn();

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UShipMovementComponent* GetShipMovement() const { return ShipMovement; }
	const UShipClassDef* GetShipClass() const { return ShipClass; }

protected:
	/** Which hull this is. Drives the flight model; assign per-Blueprint or per-instance. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ship")
	TObjectPtr<UShipClassDef> ShipClass;

	/** Which controls this ship answers to. Absent means the ship cannot be piloted. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UShipInputConfig> InputConfig;

private:
	/** Pushes the ship class's resolved flight model onto the movement component. */
	void ApplyShipClass();

	/** Sends the cached look/roll axes down as one rotation command. */
	void PushRotationInput();

	void HandleThrottle(const FInputActionValue& Value);
	void HandleStrafe(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	void HandleRoll(const FInputActionValue& Value);
	void HandleBoost(const FInputActionValue& Value);

	/** Collision lives on a simple primitive, not the art — the hull mesh is swappable. */
	UPROPERTY(VisibleAnywhere, Category = "Ship")
	TObjectPtr<USphereComponent> CollisionRoot;

	UPROPERTY(VisibleAnywhere, Category = "Ship")
	TObjectPtr<UStaticMeshComponent> Hull;

	UPROPERTY(VisibleAnywhere, Category = "Ship")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Ship")
	TObjectPtr<UCameraComponent> ChaseCamera;

	UPROPERTY(VisibleAnywhere, Category = "Ship")
	TObjectPtr<UShipMovementComponent> ShipMovement;

	/** Look and roll arrive as separate actions but leave as one command. */
	FVector2D LookInput = FVector2D::ZeroVector;
	double RollInput = 0.0;
};
