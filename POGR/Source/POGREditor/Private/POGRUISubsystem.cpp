#include "POGRUISubsystem.h"

#if WITH_EDITOR
void UPOGRUISubsystem::SetTabId(FName TabId)
{
    WidgetTabId = TabId;
}
#endif