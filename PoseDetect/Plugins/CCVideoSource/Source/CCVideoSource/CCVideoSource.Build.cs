// Created by wangpeimin@corp.netease.com

using UnrealBuildTool;
using System.IO;


public class CCVideoSource : ModuleRules
{
	private string ThirdPartyPath
	{
		get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../ThirdParty/")); }
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


    private int CopyRuntimeDependencies(string FileName, ReadOnlyTargetRules Target)
    {
		string PlatformString = Target.Platform.ToString();
		string FilePath = Path.GetFullPath(Path.Combine(ThirdPartyPath, "lib", FileName));
		CopyToProjectBinaries(FilePath, Target);
        string FilePathBinaries = Path.GetFullPath(Path.Combine(GetUProjectPath(), "Binaries", PlatformString, FileName));
        RuntimeDependencies.Add(FilePathBinaries);
		return 0;
    }

    public bool LoadThirdParty(ReadOnlyTargetRules Target)
	{
		return true;
	}

	public CCVideoSource(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		bEnableUndefinedIdentifierWarnings = false;
		CppStandard = CppStandardVersion.Cpp17;

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
				"CCActorLib"
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
                "RenderCore",
                "RHI",
                "Renderer",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		LoadThirdParty(Target);
	}
}
