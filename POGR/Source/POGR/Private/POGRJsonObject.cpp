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