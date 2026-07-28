// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#include "Hyperlane/Simulation/GalaxyClock.h"

#include "Hyperlane/Core/HyperlaneLogging.h"

int32 FGalaxyClock::Advance(double RealDeltaSeconds, TFunctionRef<void(const FGalacticDateTime&)> OnSimTick)
{
	// Negative frame time is a caller bug; recoverable, so ensure not check.
	if (!ensure(RealDeltaSeconds >= 0.0))
	{
		return 0;
	}

	Accumulator += RealDeltaSeconds * TimeScale;

	int32 TicksEmitted = 0;
	while (Accumulator >= FixedStepSeconds && TicksEmitted < MaxTicksPerAdvance)
	{
		Accumulator -= FixedStepSeconds;
		CurrentTime.AdvanceSeconds(SimSecondsPerTick);
		++TicksEmitted;
		OnSimTick(CurrentTime);
	}

	// A stall (debugger, app suspend) can leave more backlog than we are
	// willing to replay in one frame. Sim time simply advances less than
	// wall-clock time did — dropped, not deferred, or the backlog would
	// replay as a burst over the following frames.
	if (Accumulator >= FixedStepSeconds)
	{
		const int32 DroppedTicks = static_cast<int32>(Accumulator / FixedStepSeconds);
		Accumulator = FMath::Fmod(Accumulator, FixedStepSeconds);
		UE_LOG(LogGalaxySim, Warning,
			TEXT("Sim clock dropped %d backlogged ticks after a stall (max %d per advance)."),
			DroppedTicks, MaxTicksPerAdvance);
	}

	return TicksEmitted;
}

void FGalaxyClock::SetTimeScale(double InTimeScale)
{
	if (!ensure(InTimeScale >= 0.0))
	{
		return;
	}

	if (TimeScale != InTimeScale)
	{
		UE_LOG(LogGalaxySim, Log, TEXT("Sim time scale %g -> %g"), TimeScale, InTimeScale);
		TimeScale = InTimeScale;
	}
}
