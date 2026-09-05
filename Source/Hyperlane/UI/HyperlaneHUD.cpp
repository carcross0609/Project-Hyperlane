// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#include "Hyperlane/UI/HyperlaneHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/GameInstance.h"
#include "Hyperlane/Gameplay/Interaction/DockingComponent.h"
#include "Hyperlane/Gameplay/Interaction/DockingPortComponent.h"
#include "Hyperlane/Gameplay/Ships/ShipMovementComponent.h"
#include "Hyperlane/Gameplay/Ships/ShipPawn.h"
#include "Hyperlane/Simulation/Data/ShipClassDef.h"
#include "Hyperlane/Simulation/GalaxySubsystem.h"

namespace
{
	constexpr float TelemetryOriginX = 48.0f;
	constexpr float TelemetryOriginY = 48.0f;
	constexpr float TelemetryLineHeight = 18.0f;

	constexpr float SpeedBarWidth = 220.0f;
	constexpr float SpeedBarHeight = 10.0f;

	const FLinearColor TelemetryColor(0.65f, 0.85f, 1.0f, 1.0f);
	const FLinearColor BoostColor(1.0f, 0.72f, 0.25f, 1.0f);
	const FLinearColor PromptColor(0.55f, 1.0f, 0.62f, 1.0f);

	const TCHAR* DockStateLabel(EDockState State)
	{
		switch (State)
		{
		case EDockState::InRange:   return TEXT("CLEARANCE GRANTED");
		case EDockState::Docking:   return TEXT("APPROACH");
		case EDockState::Docked:    return TEXT("BERTHED");
		case EDockState::Undocking: return TEXT("DEPARTING");
		default:                    return TEXT("");
		}
	}
}

void AHyperlaneHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	float CursorY = TelemetryOriginY;

	// The sim runs whether or not a ship exists to watch it, so galactic time
	// is drawn independently of the pawn — that separation is the whole point
	// of Pillar 1 and should be visible from the first debug screen.
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (const UGalaxySubsystem* Galaxy = GameInstance->GetSubsystem<UGalaxySubsystem>())
		{
			DrawTelemetryLine(FString::Printf(TEXT("GALACTIC  %s"), *Galaxy->GetGalacticTime().ToString()), CursorY);
			DrawTelemetryLine(FString::Printf(TEXT("TIMESCALE %.2fx"), Galaxy->GetTimeScale()), CursorY);
			CursorY += TelemetryLineHeight;
		}
	}

	const AShipPawn* Ship = Cast<AShipPawn>(GetOwningPawn());
	if (!Ship)
	{
		DrawTelemetryLine(TEXT("NO SHIP POSSESSED"), CursorY);
		return;
	}

	const UShipMovementComponent* Movement = Ship->GetShipMovement();
	if (!Movement)
	{
		return;
	}

	// Metres per second, because centimetres are an engine detail and nobody
	// tunes a flight model in them.
	DrawTelemetryLine(FString::Printf(TEXT("SPEED     %.0f m/s"), Movement->GetSpeed() / 100.0), CursorY);
	DrawTelemetryLine(FString::Printf(TEXT("THROTTLE  %+.0f%%"), Movement->GetThrottleInput() * 100.0), CursorY);

	if (Movement->IsBoostActive())
	{
		DrawTelemetryLine(TEXT("BOOST     ENGAGED"), CursorY);
	}

	DrawSpeedBar(Movement->GetSpeedFraction(), Movement->IsBoostActive(), CursorY);

	if (const UShipClassDef* Class = Ship->GetShipClass())
	{
		DrawTelemetryLine(FString::Printf(TEXT("HULL      %s"), *Class->GetName()), CursorY);
	}
	else
	{
		DrawTelemetryLine(TEXT("HULL      <no ship class assigned>"), CursorY);
	}

	DrawDocking(Ship, CursorY);
}

void AHyperlaneHUD::DrawDocking(const AShipPawn* Ship, float& InOutY)
{
	const UDockingComponent* Docking = Ship->GetDocking();
	if (!Docking)
	{
		return;
	}

	const EDockState State = Docking->GetDockState();
	if (State == EDockState::Free)
	{
		return;
	}

	InOutY += TelemetryLineHeight;

	FString Berth;
	if (const UDockingPortComponent* Port = Docking->GetTargetPort())
	{
		Berth = Port->PortName.IsEmpty() ? Port->GetName() : Port->PortName.ToString();
	}

	DrawTelemetryLine(FString::Printf(TEXT("DOCK      %s  %s"), DockStateLabel(State), *Berth), InOutY);

	// The prompt only appears where pressing the key would actually do
	// something, so it never invites input the state machine will ignore.
	if (State == EDockState::InRange)
	{
		DrawText(TEXT("[F] DOCK"), PromptColor, TelemetryOriginX, InOutY, GEngine->GetSmallFont());
		InOutY += TelemetryLineHeight;
	}
	else if (State == EDockState::Docked)
	{
		DrawText(TEXT("[F] UNDOCK"), PromptColor, TelemetryOriginX, InOutY, GEngine->GetSmallFont());
		InOutY += TelemetryLineHeight;
	}
}

void AHyperlaneHUD::DrawTelemetryLine(const FString& Text, float& InOutY)
{
	DrawText(Text, TelemetryColor, TelemetryOriginX, InOutY, GEngine->GetSmallFont());
	InOutY += TelemetryLineHeight;
}

void AHyperlaneHUD::DrawSpeedBar(double Fraction, bool bBoostActive, float& InOutY)
{
	// Clamped for drawing only — the readout above still shows the true number
	// if tuning ever pushes speed past the model's own ceiling.
	const float Filled = static_cast<float>(FMath::Clamp(Fraction, 0.0, 1.0)) * SpeedBarWidth;

	DrawRect(FLinearColor(0.05f, 0.07f, 0.10f, 0.65f), TelemetryOriginX, InOutY, SpeedBarWidth, SpeedBarHeight);
	DrawRect(bBoostActive ? BoostColor : TelemetryColor, TelemetryOriginX, InOutY, Filled, SpeedBarHeight);

	InOutY += SpeedBarHeight + TelemetryLineHeight;
}
