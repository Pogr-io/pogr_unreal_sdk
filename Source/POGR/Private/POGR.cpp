// Copyright Epic Games, Inc. All Rights Reserved.

#include "POGR.h"

#if WITH_EDITOR
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#endif

#define LOCTEXT_NAMESPACE "FPOGRModule"

void FPOGRModule::StartupModule()
{
	RegisterPOGREndpointSettings();
}

void FPOGRModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	UnregisterPOGREndpointSettings();
}

void FPOGRModule::RegisterPOGREndpointSettings()
{
#if WITH_EDITOR
	ModuleSettings = NewObject<UPOGREndpointSettings>(GetTransientPackage(), "POGR", RF_Standalone);
	ModuleSettings->AddToRoot();
	//Register Settings
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "POGR",
			LOCTEXT("RuntimeSettingsName", "POGR"),
			LOCTEXT("RuntimeSettingsDescription", "Configure POGR plugin settings"),
			ModuleSettings);
	}
#endif
}

void FPOGRModule::UnregisterPOGREndpointSettings()
{
#if WITH_EDITOR
	// Unregister settings
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "POGR");
	}

	if (!GExitPurge)
	{
		ModuleSettings->RemoveFromRoot();
	}
	else
	{
		ModuleSettings = nullptr;
	}
#endif
}

UPOGREndpointSettings* FPOGRModule::GetSettings() const
{
	check(ModuleSettings);
	return ModuleSettings;
}
#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPOGRModule, POGR)