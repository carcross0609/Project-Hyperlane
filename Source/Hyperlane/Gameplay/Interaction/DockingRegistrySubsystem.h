// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DockingRegistrySubsystem.generated.h"

class UDockingPortComponent;
class UShipSizeClassDef;

/**
 * Every docking port currently in the level, and the only sanctioned way to
 * find one.
 *
 * *Why this exists at all:* discovery via GetAllActorsOfClass is banned
 * (TechnicalArchitecture §6). Overlap volumes were the alternative, but that
 * spends collision channels and physics ticks on what is fundamentally a
 * proximity question, and it makes "which berth is best" hard to answer —
 * overlaps report entry, not ranking.
 *
 * *Why World scope, not GameInstance:* ports belong to a level. A
 * GameInstance subsystem would survive a level load and leak stale berths
 * into the next system.
 *
 * The player has no privileged path in here. An NPC freighter deciding where
 * to deliver cargo asks the same question with the same call, which is the
 * property TechnicalArchitecture §7 asks for: if a ship component ever needs
 * to know whether it is the player, the design is wrong.
 */
UCLASS()
class HYPERLANE_API UDockingRegistrySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void RegisterPort(UDockingPortComponent* Port);
	void UnregisterPort(UDockingPortComponent* Port);

	/**
	 * Nearest berth that will take this hull and is not already claimed, or
	 * null. Distance is measured to the berth itself, and a port only counts
	 * inside its own ApproachRadius — a big station reaches further than a
	 * landing pad, which is authored per port rather than assumed here.
	 */
	UDockingPortComponent* FindBestPortFor(const FVector& ShipLocation,
		const UShipSizeClassDef* SizeClass,
		const AActor* RequestingShip) const;

	/** For debug views; ports may have died since registering, so entries can be stale. */
	const TArray<TWeakObjectPtr<UDockingPortComponent>>& GetPorts() const { return Ports; }

private:
	/**
	 * Weak by design: a station can be destroyed without telling us, and the
	 * registry must never be the reason a port stays alive.
	 */
	TArray<TWeakObjectPtr<UDockingPortComponent>> Ports;
};
