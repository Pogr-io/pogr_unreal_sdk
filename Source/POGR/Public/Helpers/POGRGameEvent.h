#pragma once

#include "CoreMinimal.h"
#include "POGRGameEvent.generated.h"

USTRUCT(BlueprintType)
struct FPlayerEventData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString player_id;

    UPROPERTY(BlueprintReadWrite)
    FString achievement_name;
};

USTRUCT(BlueprintType)
struct FGameEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString event;

    UPROPERTY(BlueprintReadWrite)
    FString sub_event;

    UPROPERTY(BlueprintReadWrite)
    FString event_type;

    UPROPERTY(BlueprintReadWrite)
    FString event_flag;

    UPROPERTY(BlueprintReadWrite)
    FString event_key;

    UPROPERTY(BlueprintReadWrite)
    FPlayerEventData event_data;
};
