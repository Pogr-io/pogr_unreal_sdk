#pragma once

#include "CoreMinimal.h"
#include "POGRGameData.generated.h"

USTRUCT(BlueprintType)
struct FOrganizationGameData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString UUID;
	FString StudioUUID;

	UPROPERTY(BlueprintReadOnly)
	FString GameTitle;

	UPROPERTY(BlueprintReadOnly)
	FString URL;
	FString CreatedOn;

	UPROPERTY(BlueprintReadOnly)
	UTexture2D* GameImage;

	bool operator==(const FOrganizationGameData& Other) const
	{
		return UUID == Other.UUID && StudioUUID == Other.StudioUUID && GameTitle == Other.GameTitle && CreatedOn == Other.CreatedOn && URL == Other.URL;
	}
};