// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using UnrealBuildTool;
using System.IO;

public class CCActorLib : ModuleRules
{
    private string LibPath
    {
        get
        {
            return Path.GetFullPath(Path.Combine(PluginDirectory, "LIB"));
        }
    }

    public string GetUProjectPath()
    {
        return Path.Combine(ModuleDirectory, "../../../..");
    }


    private int HashFile(string FilePath)
    {
        string DLLString = File.ReadAllText(FilePath);
        return DLLString.GetHashCode() + DLLString.Length;  //ensure both hash and file lengths match
    }

    private void CopyToProjectBinaries(string Filepath, ReadOnlyTargetRules Target)
    {
        string BinariesDir = Path.Combine(GetUProjectPath(), "Binaries", Target.Platform.ToString());
        string Filename = Path.GetFileName(Filepath);

        //convert relative path 
        string FullBinariesDir = Path.GetFullPath(BinariesDir);

        if (!Directory.Exists(FullBinariesDir))
        {
            Directory.CreateDirectory(FullBinariesDir);
        }

        string FullExistingPath = Path.Combine(FullBinariesDir, Filename);
        bool ValidFile = false;

        //File exists, check if they're the same
        if (File.Exists(FullExistingPath))
        {
            int ExistingFileHash = HashFile(FullExistingPath);
            int TargetFileHash = HashFile(Filepath);
            ValidFile = ExistingFileHash == TargetFileHash;
            if (!ValidFile)
            {
            }
        }
        if (!ValidFile)
        {
            File.Copy(Filepath, Path.Combine(FullBinariesDir, Filename), true);
        }
    }

    public CCActorLib(ReadOnlyTargetRules Target) : base(Target)
	{
        //PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        DefaultBuildSettings = BuildSettingsVersion.V1;
        PCHUsage = ModuleRules.PCHUsageMode.NoPCHs;
        CppStandard = CppStandardVersion.Cpp17;
        bEnableExceptions = true;
        bEnableUndefinedIdentifierWarnings = false;

		PublicSystemLibraries.Add("d3d11.lib");

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
				"RenderCore", 
				"RHI",
				"Renderer", 
				"Projects",
				"Json"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		PublicAdditionalLibraries.AddRange(new string[] {
			Path.Combine(LibPath, "bass/bass.lib"),
            //Path.Combine(LibPath, "libyuv/yuv.lib"),
            //Path.Combine(LibPath, "libyuv/jpeg.lib"),
            Path.Combine(LibPath, "libyuv/jpeg-static.lib"),
           // Path.Combine(LibPath, "libyuv/turbojpeg.lib"),
            Path.Combine(LibPath, "libyuv/turbojpeg-static.lib"),
        });

        Definitions.Add("HAVE_JPEG");

        PublicSystemIncludePaths.Add(Path.Combine(PluginDirectory, "Include"));

        string PlatformString = Target.Platform.ToString();
        RuntimeDependencies.Add("$(TargetOutputDir)/bass.dll", Path.Combine(LibPath, "bass/bass.dll"));
    }
}
