// Copyright SAEVUS. All Rights Reserved.

using UnrealBuildTool;

public class SAEVUS : ModuleRules
{
	public SAEVUS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Lets sources include by feature folder, e.g. "Movement/SAEVUSMovementComponent.h".
		PublicIncludePaths.Add(ModuleDirectory);

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
