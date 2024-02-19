#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"

#include "POGRUISubsystem.generated.h"

UCLASS()
class UPOGRUISubsystem : public UEditorSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "POGRUISubsystem")
    void SetTabId(FName TabId);

private:
    UFUNCTION(BlueprintCallable, Category = "POGRUISubsystem")
    const FName GetTabId() const { return WidgetTabId; }

private:
    FName WidgetTabId;
};