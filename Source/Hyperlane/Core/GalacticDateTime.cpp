// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

#include "Core/GalacticDateTime.h"

void FGalacticDateTime::AdvanceSeconds(int64 DeltaSeconds)
{
	// Sim time is monotonic by design; a negative delta means a caller's
	// math is wrong, not a valid request. Recoverable, so ensure not check.
	if (!ensure(DeltaSeconds >= 0))
	{
		return;
	}

	TotalSimSeconds += DeltaSeconds;
}

FString FGalacticDateTime::ToString() const
{
	return FString::Printf(TEXT("Day %lld, %02d:%02d:%02d"),
		GetDay(), GetHour(), GetMinute(), GetSecond());
}
