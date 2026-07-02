// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#pragma once

#include "CoreMinimal.h"
#include "GalacticDateTime.generated.h"

/**
 * Simulation time as a first-class type (TechnicalArchitecture §4.2).
 * Never accumulate sim time as ad-hoc floats — floats drift, and drifted
 * time desynchronizes every system that keys off it.
 *
 * Stored as whole sim-seconds since the campaign epoch (start of the
 * simulation). Calendar semantics are deliberately plain day/hour/minute/
 * second for now; a Star Wars calendar (if ever wanted) is presentation,
 * layered on top — the sim itself stays generic (Pillar 6).
 *
 * Persistence: serialized through FHyperlaneSaveSchema when that arrives
 * (Phase 2, TechnicalArchitecture §8); the schema carries the version.
 */
USTRUCT(BlueprintType)
struct FGalacticDateTime
{
	GENERATED_BODY()

	FGalacticDateTime() = default;

	explicit FGalacticDateTime(int64 InTotalSimSeconds)
		: TotalSimSeconds(InTotalSimSeconds)
	{
	}

	static constexpr int64 SecondsPerMinute = 60;
	static constexpr int64 SecondsPerHour   = 60 * SecondsPerMinute;
	static constexpr int64 SecondsPerDay    = 24 * SecondsPerHour;

	/** Advances time forward. Negative deltas are rejected — sim time never rewinds. */
	void AdvanceSeconds(int64 DeltaSeconds);

	int64 GetTotalSimSeconds() const { return TotalSimSeconds; }

	/** Whole days elapsed since the campaign epoch. */
	int64 GetDay() const { return TotalSimSeconds / SecondsPerDay; }

	/** Hour of the current day, 0–23. */
	int32 GetHour() const { return static_cast<int32>((TotalSimSeconds % SecondsPerDay) / SecondsPerHour); }

	/** Minute of the current hour, 0–59. */
	int32 GetMinute() const { return static_cast<int32>((TotalSimSeconds % SecondsPerHour) / SecondsPerMinute); }

	/** Second of the current minute, 0–59. */
	int32 GetSecond() const { return static_cast<int32>(TotalSimSeconds % SecondsPerMinute); }

	/** Human-readable form for logs and debug UI, e.g. "Day 12, 03:45:07". */
	FString ToString() const;

	bool operator==(const FGalacticDateTime& Other) const { return TotalSimSeconds == Other.TotalSimSeconds; }
	bool operator!=(const FGalacticDateTime& Other) const { return TotalSimSeconds != Other.TotalSimSeconds; }
	bool operator<(const FGalacticDateTime& Other) const { return TotalSimSeconds < Other.TotalSimSeconds; }
	bool operator<=(const FGalacticDateTime& Other) const { return TotalSimSeconds <= Other.TotalSimSeconds; }
	bool operator>(const FGalacticDateTime& Other) const { return TotalSimSeconds > Other.TotalSimSeconds; }
	bool operator>=(const FGalacticDateTime& Other) const { return TotalSimSeconds >= Other.TotalSimSeconds; }

private:
	/** Whole sim-seconds since the campaign epoch. */
	UPROPERTY(SaveGame)
	int64 TotalSimSeconds = 0;
};
