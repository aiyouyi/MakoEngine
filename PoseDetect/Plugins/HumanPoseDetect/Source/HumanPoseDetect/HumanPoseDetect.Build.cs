// Some copyright should be here...

using UnrealBuildTool;
using System.IO;

public class HumanPoseDetect : ModuleRules
{
    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ThirdParty/")); }
    }

    public HumanPoseDetect(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		PublicIncludePaths.Add(Path.GetFullPath(Path.Combine(ThirdPartyPath, "include/pose_service")));
        PublicSystemLibraryPaths.Add(Path.GetFullPath(Path.Combine(ThirdPartyPath, "lib")));
        PublicSystemLibraries.Add("pose_service.lib");
		RuntimeDependencies.Add("$(TargetOutputDir)/MNN.dll", Path.Combine(ThirdPartyPath, "bin/MNN.dll"));
		RuntimeDependencies.Add("$(TargetOutputDir)/opencv_world410.dll", Path.Combine(ThirdPartyPath, "bin/opencv_world410.dll"));
		RuntimeDependencies.Add("$(TargetOutputDir)/pose_service.dll", Path.Combine(ThirdPartyPath, "bin/pose_service.dll"));
		RuntimeDependencies.Add("$(TargetOutputDir)/qsdeft.pt", Path.Combine(ThirdPartyPath, "bin/qsdeft.pt"));
	}
}
