// Copyright Epic Games, Inc. All Rights Reserved.

#include "POGREditor.h"
#include "POGRStyle.h"
#include "POGRCommands.h"
#include "POGRUISubsystem.h"
#include "Misc/MessageDialog.h"

#if WITH_EDITOR
#include "ToolMenus.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "EditorUtilitySubsystem.h"
#include "EditorAssetLibrary.h"
#endif

#if WITH_EDITOR
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#endif

static const FName POGRTabName("POGR");

#define LOCTEXT_NAMESPACE "FPOGREditorModule"

void FPOGREditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FPOGRStyle::Initialize();
	FPOGRStyle::ReloadTextures();

	FPOGRCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FPOGRCommands::Get().OpenPOGRWindow,
		FExecuteAction::CreateRaw(this, &FPOGREditorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FPOGREditorModule::RegisterMenus));
}

void FPOGREditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FPOGRStyle::Shutdown();

	FPOGRCommands::Unregister();
}

void FPOGREditorModule::PluginButtonClicked()
{
	SpawnPluginUtilityWidget();
}

void FPOGREditorModule::RegisterMenus()
{
#if WITH_EDITOR
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
#endif
}

void FPOGREditorModule::SpawnPluginUtilityWidget()
{
#if WITH_EDITOR
	UObject* Blueprint = UEditorAssetLibrary::LoadAsset(TEXT("/POGR/UMG/EUW_PreSignIn.EUW_PreSignIn"));
	if (IsValid(Blueprint)) {
		UEditorUtilityWidgetBlueprint* EditorWidget = Cast<UEditorUtilityWidgetBlueprint>(Blueprint);
		if (EditorWidget) {
			UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
			UPOGRUISubsystem* POGRUISubsystem = GEditor->GetEditorSubsystem<UPOGRUISubsystem>();
			FName TabId = FName();
			EditorUtilitySubsystem->SpawnAndRegisterTabAndGetID(EditorWidget, TabId);
			POGRUISubsystem->SetTabId(TabId);
		}
	}
#endif
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPOGREditorModule, POGREditor)