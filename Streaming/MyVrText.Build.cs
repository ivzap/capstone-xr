// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class MyVrTest : ModuleRules
{
	public MyVrTest(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        // Add Public Include Paths
        PublicIncludePaths.AddRange(new string[] {
            "Plugins/glTFRuntime/Source/glTFRuntime/Public"
        });

        // Add Private Include Paths
        PrivateIncludePaths.AddRange(new string[] {
            "Plugins/glTFRuntime/Source/glTFRuntime/Private"
        });

        PublicDependencyModuleNames.AddRange(new string[] { "HTTP", "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "glTFRuntime" });
    }
}
