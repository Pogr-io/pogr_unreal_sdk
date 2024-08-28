#pragma once

#include "CoreMinimal.h"
#include "POGRGameMetrics.generated.h"

USTRUCT(BlueprintType)
struct FGameMetricsData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 players_online = 0;

    UPROPERTY(BlueprintReadWrite)
    float average_latency_ms = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float server_load_percentage = 0.0f;
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