// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HyperlaneHUD.generated.h"

/**
 * Debug telemetry drawn straight to the canvas (Roadmap Phase 1: "ugly is
 * fine, diegetic comes later").
 *
 * Deliberately not UMG. Phase 1's job is to make flight *tunable*, and a
 * canvas readout needs no widget asset, no designer round-trip, and cannot
 * break when the project has no content yet. The real cockpit instruments
 * replace this entirely rather than growing out of it.
 *
 * Reads downward only: it queries the pawn and the sim, and neither knows
 * it exists (TechnicalArchitecture §2).
 */
UCLASS()
class HYPERLANE_API AHyperlaneHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	/** Draws one line and advances the cursor, so callers stay a readable list. */
	void DrawTelemetryLine(const FString& Text, float& InOutY);

	/** A crude filled bar; the fastest way to *feel* a tuning change. */
	void DrawSpeedBar(double Fraction, bool bBoostActive, float& InOutY);

	/** Docking state and its prompt. Silent while nothing is in reach. */
	void DrawDocking(const class AShipPawn* Ship, float& InOutY);
};
