#pragma once

#include "CoreMinimal.h"
#include "POGRGameLogs.generated.h"

USTRUCT(BlueprintType)
struct FGameLogData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString user_id;

    UPROPERTY(BlueprintReadWrite)
    FString timestamp;

    UPROPERTY(BlueprintReadWrite)
    FString ip_address;
};

USTRUCT(BlueprintType)
struct FGameLogTags
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString system;

    UPROPERTY(BlueprintReadWrite)
    FString action;
};

USTRUCT(BlueprintType)
struct FGameLog
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString service;

    UPROPERTY(BlueprintReadWrite)
    FString environment;

    UPROPERTY(BlueprintReadWrite)
    FString severity;

    UPROPERTY(BlueprintReadWrite)
    FString type;

    UPROPERTY(BlueprintReadWrite)
    FString log;

    UPROPERTY(BlueprintReadWrite)
    FGameLogData data;

    UPROPERTY(BlueprintReadWrite)
    FGameLogTags tags;
};