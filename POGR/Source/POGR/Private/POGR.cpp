// Copyright Epic Games, Inc. All Rights Reserved.

#include "POGR.h"
#include "POGRStyle.h"
#include "POGRCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "EditorUtilitySubsystem.h"
#include "EditorAssetLibrary.h"
#include "POGRSubsystem.h"

#if WITH_EDITOR
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#endif

static const FName POGRTabName("POGR");

#define LOCTEXT_NAMESPACE "FPOGRModule"

void FPOGRModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FPOGRStyle::Initialize();
	FPOGRStyle::ReloadTextures();

	FPOGRCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FPOGRCommands::Get().OpenPOGRWindow,
		FExecuteAction::CreateRaw(this, &FPOGRModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FPOGRModule::RegisterMenus));
	
	RegisterPOGREndpointSettings();
}

void FPOGRModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FPOGRStyle::Shutdown();

	FPOGRCommands::Unregister();

	UnregisterPOGREndpointSettings();
}

void FPOGRModule::PluginButtonClicked()
{
	SpawnPluginUtilityWidget();
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

void FPOGRModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FPOGRCommands::Get().OpenPOGRWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FPOGRCommands::Get().OpenPOGRWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

void FPOGRModule::SpawnPluginUtilityWidget()
{
	UObject* Blueprint = UEditorAssetLibrary::LoadAsset(TEXT("/POGR/UMG/EUW_PreSignIn.EUW_PreSignIn"));
	if (IsValid(Blueprint)) {
		UEditorUtilityWidgetBlueprint* EditorWidget = Cast<UEditorUtilityWidgetBlueprint>(Blueprint);
		if (EditorWidget) {
			UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
			UPOGRSubsystem* POGRUISubsystem = GEditor->GetEditorSubsystem<UPOGRSubsystem>();
			FName TabId = FName();
			EditorUtilitySubsystem->SpawnAndRegisterTabAndGetID(EditorWidget, TabId);
			POGRUISubsystem->SetTabId(TabId);
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPOGRModule, POGR)