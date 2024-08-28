#pragma once

#include "CoreMinimal.h"
#include "POGROrganizationData.generated.h"

USTRUCT(BlueprintType)
struct FOrganizationData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString UUID;

	UPROPERTY(BlueprintReadOnly)
	FString Name;
	FString CreatedOn;
	FString Type;

	UPROPERTY(BlueprintReadOnly)
	FString URL;

	UPROPERTY(BlueprintReadOnly)
	UTexture2D* OrganizationImage;

	bool operator==(const FOrganizationData& Other) const
	{
		return UUID == Other.UUID && Name == Other.Name && CreatedOn == Other.CreatedOn && Type == Other.Type && URL == Other.URL;
	}
};