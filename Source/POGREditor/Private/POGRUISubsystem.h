#pragma once

#include "CoreMinimal.h"

#if WITH_EDITOR
#include "EditorSubsystem.h"
#endif

#include "POGRUISubsystem.generated.h"

#if WITH_EDITOR
UCLASS()
class UPOGRUISubsystem : public UEditorSubsystem
{
	GENERATED_BODY()
/*
	* POGR UI Helper Function *
*/
public:
	UFUNCTION(BlueprintCallable, Category = "POGR UI Subsystem")
	void SetTabId(FName TabId);

private:
	UFUNCTION(BlueprintCallable, Category = "POGR UI Subsystem")
	const FName GetTabId() const { return WidgetTabId; }

private:
	FName WidgetTabId;
};
#endif