// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

using UnrealBuildTool;
using System.Collections.Generic;

public class HyperlaneEditorTarget : TargetRules
{
	public HyperlaneEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Hyperlane");
	}
}
