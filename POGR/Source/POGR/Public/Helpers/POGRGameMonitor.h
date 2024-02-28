#pragma once

#include "CoreMinimal.h"
#include "POGRGameMonitor.generated.h"

USTRUCT(BlueprintType)
struct FGameSystemSettings
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString graphics_quality;
};


USTRUCT(BlueprintType)
struct FGameSystemMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    float cpu_usage;

    UPROPERTY(BlueprintReadWrite)
    float memory_usage;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> dlls_loaded;

    UPROPERTY(BlueprintReadWrite)
    FGameSystemSettings settings;
};