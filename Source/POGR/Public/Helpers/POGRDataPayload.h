#pragma once

#include "CoreMinimal.h"
#include "POGRURLFilters.h"
#include "POGRDataPayload.generated.h"

USTRUCT(BlueprintType)
struct FDataPayload
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Id;

	UPROPERTY(BlueprintReadWrite)
	FString ClientId;

	UPROPERTY(BlueprintReadWrite)
	FString BuildId;

	UPROPERTY(BlueprintReadWrite)
	TEnumAsByte<EAcceptedStatus> AcceptedStatus;

	UPROPERTY(BlueprintReadWrite)
	FString DataReceivedDate;

	UPROPERTY(BlueprintReadWrite)
	int32 Calls;
};