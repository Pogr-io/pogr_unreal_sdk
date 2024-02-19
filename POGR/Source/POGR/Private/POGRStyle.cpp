// Copyright Epic Games, Inc. All Rights Reserved.

#include "POGRStyle.h"
#include "POGR.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FPOGRStyle::StyleInstance = nullptr;

void FPOGRStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FPOGRStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FPOGRStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("POGRStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);

TSharedRef< FSlateStyleSet > FPOGRStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("POGRStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("POGR")->GetBaseDir() / TEXT("Resources"));

	Style->Set("POGR.OpenPOGRWindow", new IMAGE_BRUSH(TEXT("Icon/POGRLOGO"), Icon40x40));
	return Style;
}

void FPOGRStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FPOGRStyle::Get()
{
	return *StyleInstance;
}
