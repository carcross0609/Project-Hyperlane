// Copyright (c) 2026 Carson Crossno. Personal, non-commercial project.

using UnrealBuildTool;

public class Hyperlane : ModuleRules
{
	public Hyperlane(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Dependencies are added when first used, not speculatively
		// (CodingStandards: includes minimal).
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput"
		});
	}
}
