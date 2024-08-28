#pragma once

#include "CoreMinimal.h"
#include "POGRProfileData.generated.h"

USTRUCT(BlueprintType)
struct FUserProfileData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString UserName;
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly)
	FString AvatarURL;
	int32 Level;
	int32 Exp;
	int32 RequiredExp;

	UPROPERTY(BlueprintReadOnly)
	UTexture2DDynamic* ProfileImage;
};