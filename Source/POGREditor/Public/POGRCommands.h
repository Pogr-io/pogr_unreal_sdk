// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "POGRStyle.h"

class FPOGRCommands : public TCommands<FPOGRCommands>
{
public:

	FPOGRCommands()
		: TCommands<FPOGRCommands>(TEXT("POGR"), NSLOCTEXT("Contexts", "POGR", "POGR Plugin"), NAME_None, FPOGRStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPOGRWindow;
};
