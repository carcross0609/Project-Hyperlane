// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

using UnrealBuildTool;
using System.Collections.Generic;

public class HyperlaneTarget : TargetRules
{
	public HyperlaneTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Hyperlane");
	}
}
