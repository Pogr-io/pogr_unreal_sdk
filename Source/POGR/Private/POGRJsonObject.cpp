#include "POGRJsonObject.h"

void UJsonRequestObject::ConstructJsonObject()
{
    JsonObject = MakeShared<FJsonObject>();
}

void UJsonRequestObject::SetStringField(const FString& FieldName, const FString& StringValue)
{
    if (FieldName.IsEmpty())
    {
        return;
    }

    JsonObject->SetStringField(FieldName, StringValue);
}

void UJsonRequestObject::SetBoolField(const FString& FieldName, bool InValue)
{
    if (FieldName.IsEmpty())
    {
        return;
    }

    JsonObject->SetBoolField(FieldName, InValue);
}

void UJsonRequestObject::SetNumberField(const FString& FieldName, double Number)
{
    if (FieldName.IsEmpty())
    {
        return;
    }

    JsonObject->SetNumberField(FieldName, Number);
}

void UJsonRequestObject::SetArrayField(const FString& FieldName, const TArray<FString>& Array)
{
    if (FieldName.IsEmpty())
    {
        return;
    }

    JsonObject->SetArrayField(FieldName, StringArrayToJsonValues(Array));
}

void UJsonRequestObject::SetObjectField(const FString& FieldName, const UJsonRequestObject* SubJsonObject)
{
    if (FieldName.IsEmpty())
    {
        return;
    }

    if (SubJsonObject)
    {
        FString SubJsonString;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&SubJsonString);
        FJsonSerializer::Serialize(SubJsonObject->GetJsonRequestObject().ToSharedRef(), Writer);
        JsonObject->SetStringField(FieldName, SubJsonString);
    }
}

TArray<TSharedPtr<FJsonValue>> UJsonRequestObject::StringArrayToJsonValues(const TArray<FString>& StringArray)
{
    TArray<TSharedPtr<FJsonValue>> JsonValues;
    for (const FString& StringValue : StringArray)
    {
        JsonValues.Add(MakeShared<FJsonValueString>(StringValue));
    }
    return JsonValues;
}