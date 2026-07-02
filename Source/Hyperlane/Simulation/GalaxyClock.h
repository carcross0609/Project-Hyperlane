// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#pragma once

#include "CoreMinimal.h"
#include "Core/GalacticDateTime.h"

/**
 * Fixed-timestep driver for the galaxy simulation (TechnicalArchitecture §4.2).
 *
 * Plain C++ class — no UObject, no engine framework — so it is unit-testable
 * and unaware of the subsystem hosting it. Real frame time goes in;
 * zero or more fixed sim ticks come out. The sim never does work per
 * rendered frame: a fast machine and a slow machine produce the same
 * sequence of sim states.
 *
 * Time scale is a first-class parameter: debugging fast-forward and
 * hyperspace time compression are the same mechanism.
 */
class FGalaxyClock
{
public:
	FGalaxyClock() = default;

	/**
	 * Feeds real elapsed time to the clock, invoking OnSimTick for each
	 * whole fixed step crossed. The callback receives the sim time as of
	 * that tick. Returns the number of ticks emitted.
	 */
	int32 Advance(double RealDeltaSeconds, TFunctionRef<void(const FGalacticDateTime&)> OnSimTick);

	const FGalacticDateTime& GetCurrentTime() const { return CurrentTime; }

	double GetTimeScale() const { return TimeScale; }

	/** Scale ≥ 0. Zero pauses the sim; large values fast-forward it. */
	void SetTimeScale(double InTimeScale);

	/** Seconds of galactic time each sim tick represents. */
	int64 GetSimSecondsPerTick() const { return SimSecondsPerTick; }

private:
	FGalacticDateTime CurrentTime;

	/** Unconsumed scaled real time, in seconds. Always < FixedStepSeconds after Advance(). */
	double Accumulator = 0.0;

	/** Multiplier on incoming real time. 1 = real-time, 0 = paused. */
	double TimeScale = 1.0;

	// Initial tuning (TechnicalArchitecture §4.2): one sim tick per real
	// second, each advancing one galactic minute.
	// TODO(phase-3): move to data-driven sim tuning once the economy needs
	// these values designed rather than defaulted.
	static constexpr double FixedStepSeconds = 1.0;
	static constexpr int64 SimSecondsPerTick = FGalacticDateTime::SecondsPerMinute;

	/**
	 * Upper bound on ticks emitted per Advance() call. A long hitch
	 * (debugger break, app suspended) must not stall the frame catching up
	 * tick-by-tick; the remainder is dropped with a warning. Galaxy-scale
	 * work is bucketed across ticks, so no single tick may pay for a stall.
	 */
	static constexpr int32 MaxTicksPerAdvance = 10;
};
