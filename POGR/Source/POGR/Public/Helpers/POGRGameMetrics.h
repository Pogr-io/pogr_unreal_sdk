#pragma once

#include "CoreMinimal.h"
#include "POGRGameMetrics.generated.h"

USTRUCT(BlueprintType)
struct FGameMetricsData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 players_online;

    UPROPERTY(BlueprintReadWrite)
    float average_latency_ms;

    UPROPERTY(BlueprintReadWrite)
    float server_load_percentage;
};

USTRUCT(BlueprintType)
struct FGameTags
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString location;

    UPROPERTY(BlueprintReadWrite)
    FString game_mode;
};

USTRUCT(BlueprintType)
struct FGameMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString service;

    UPROPERTY(BlueprintReadWrite)
    FString environment;

    UPROPERTY(BlueprintReadWrite)
    FGameMetricsData metrics;

    UPROPERTY(BlueprintReadWrite)
    FGameTags tags;
};