// Copyright Epic Games, Inc. All Rights Reserved.

#include "CCActorLib.h"
#include "Interfaces/IPluginManager.h"
#include "ShaderCore.h"
#include <Runtime/Core/Public/Misc/Paths.h>
#include <Runtime/Core/Public/HAL/PlatformFilemanager.h>
#include <Runtime/core/Public/GenericPlatform/GenericPlatformFile.h>

#define LOCTEXT_NAMESPACE "FCCActorLibModule"

void FCCActorLibModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	TSharedPtr<IPlugin> tsp_Plugin = IPluginManager::Get().FindPlugin(TEXT("CCActorLib"));
	if (tsp_Plugin.IsValid())
	{
		FString PluginShaderDir = FPaths::Combine(tsp_Plugin->GetBaseDir(), TEXT("Shaders"));
		AddShaderSourceDirectoryMapping(TEXT("/Plugin/CCActorLib"), PluginShaderDir);
	}
}

void FCCActorLibModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCCActorLibModule, CCActorLib)