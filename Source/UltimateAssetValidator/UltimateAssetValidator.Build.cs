// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UltimateAssetValidator : ModuleRules
{
	public UltimateAssetValidator(ReadOnlyTargetRules Target) : base(
		Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.EngineDefault;
		DefaultBuildSettings = BuildSettingsVersion.Latest;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"DataValidation",
			});


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore", 
				"UnrealEd",
			});
	}
}