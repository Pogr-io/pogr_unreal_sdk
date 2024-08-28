#pragma once

#include "CoreMinimal.h"
#include "POGRURLFilters.generated.h"

UENUM(BlueprintType)
enum EAcceptedStatus : uint8
{
	Pending,
	Saved,
	Ignored,
	None
};

UENUM()
enum URLDefinition : uint8 {
	Data,
	Events,
	Logs,
	Metrics
};

UENUM()
enum URLAction : uint8 {
	List,
	Update,
	Definition,
	Receiving
};