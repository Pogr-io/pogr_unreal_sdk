// Copyright Epic Games, Inc. All Rights Reserved.

#include "POGRCommands.h"

#define LOCTEXT_NAMESPACE "FPOGRModule"

void FPOGRCommands::RegisterCommands()
{
	UI_COMMAND(OpenPOGRWindow, "POGR", "Execute POGR", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
