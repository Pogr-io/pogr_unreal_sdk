#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "Http.h"
#include "Json.h"
#include "POGRSettings.h"

#include "PerformanceSubsystem.generated.h"

class IWebSocket;

//USTRUCT(BlueprintType)
//struct FPerformanceData
//{
//	GENERATED_BODY();
//
//	FString CPUBrand;
//	FString CPUName;
//	FString GPUBrand;
//	FString OSVersion;
//
//	float UsedPhysical;
//	float AvailablePhysical;
//	float AvailableVirtual;
//	float UsedVirtual;
//	float PeakUsedVirtual;//	float PeakUsedPhysical;
//};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoginComplete, bool, bLoginStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSessionCreationCallback);

UCLASS(BlueprintType)
class UJsonRequestObject : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void ConstructJsonObject()
	{
		JsonObject = MakeShared<FJsonObject>();
	}

	UFUNCTION(BlueprintCallable)
	void SetStringField(const FString& FieldName, const FString& StringValue)
	{
		JsonObject->SetStringField(FieldName, StringValue);
	}

	UFUNCTION(BlueprintCallable)
	void SetBoolField(const FString& FieldName, bool InValue)
	{
		JsonObject->SetBoolField(FieldName, InValue);
	}

	UFUNCTION(BlueprintCallable)
	void SetNumberField(const FString& FieldName, double Number)
	{
		JsonObject->SetNumberField(FieldName, Number);
	}

	UFUNCTION(BlueprintCallable)
	void SetObjectField(const FString& FieldName, const UJsonRequestObject* SubJsonObject)
	{
		if (SubJsonObject)
		{
			FString SubJsonString;
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&SubJsonString);
			FJsonSerializer::Serialize(SubJsonObject->GetJsonRequestObject().ToSharedRef(), Writer);
			JsonObject->SetStringField(FieldName, SubJsonString);
		}
	}

	UFUNCTION(BlueprintCallable)
	void SetJsonRequestObjectName(const FString& jsonRequestObjectName)
	{
		JsonRequestObjectName = jsonRequestObjectName;
	}

	const FString GetJsonRequestObjectName() const { return JsonRequestObjectName; }
	const TSharedPtr<FJsonObject> GetJsonRequestObject() const { return JsonObject; }

private:
	TSharedPtr<FJsonObject> JsonObject;
	FString JsonRequestObjectName;
};

UCLASS(BlueprintType)
class UPerformanceSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

//public:
//	UFUNCTION(BlueprintCallable, Category = "POGRPerformanceSubsystem", meta = (DisplayName = "Performance Dump"))
//	const FPerformanceData GetPOGRPerformanceDump() const;

public:
	UFUNCTION(BlueprintPure, Category = "POGR User Helper Function")
	const FString GenerateUniqueUserAssociationId() const;

public:
	UFUNCTION(BlueprintCallable, Category = "POGRPerformanceSubsystem")
	void SendMetricsEvent(const UJsonRequestObject* jsonObject, const FString& SessionID);

	UFUNCTION(BlueprintCallable, Category = "POGRPerformanceSubsystem")
	void SendInitSessionEvent(const FString& ClientId, const FString& BuildId, const FString& AssociationId);

	UFUNCTION(BlueprintCallable, Category = "POGRPerformanceSubsystem")
	void SendEndSessionEvent(const FString& SessionId, const UJsonRequestObject* JsonObjectValue);

	UFUNCTION(BlueprintCallable, Category = "POGRPerformanceSubsystem")
	void SendEvent(const FString& SessionId);

	UFUNCTION(BlueprintCallable, Category = "POGRPerformanceSubsystem")
	void SendDataEvent(const FString& SessionId);

	UFUNCTION(BlueprintCallable, Category = "POGRPerformanceSubsystem")
	void SendLogsEvent(const FString& SessionId);

	UFUNCTION(BlueprintCallable, Category = "POGRPerformanceSubsystem")
	void SendMonitorEvent(const FString& SessionId);

	UFUNCTION(BlueprintPure, Category = "POGRPerformanceSubsystem")
	const FString GetSessionId() const { return SessionId; };

	UFUNCTION(BlueprintPure, Category = "POGRPerformanceSubsystem")
	const FString GetAccessTokken() const { return AccessTokken; };

	UFUNCTION(BlueprintPure, Category = "POGRPerformanceSubsystem")
	const bool GetIsUserLoggedIn() const { return IsLoggedIn; }

	UFUNCTION(BlueprintPure, Category = "POGRPerformanceSubsystem")
	UJsonRequestObject* GetJsonRequestObject();

	/* 
		*Testing Function 
	*/
	UFUNCTION(BlueprintCallable, Category = "Test")
	void SendTestMethod(const UJsonRequestObject* jsonObject);

	UFUNCTION(BlueprintCallable, Category = "Test")
	void SendTestEvent(const TArray<UJsonRequestObject*> jsonObject);

	UFUNCTION(BlueprintCallable, Category = "POGR Subsystem", meta = (Latent, LatentInfo = "LatentInfo", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	void SendDataEventAsync(const TArray<UJsonRequestObject*> jsonObject);
	/******************************************/

public:
	UFUNCTION(BlueprintPure, Category = "Performance Subsystem")
	const FString GetCPUName() const;

	UFUNCTION(BlueprintPure, Category = "Performance Subsystem")
	const FString GetCPUBrand() const;

	UFUNCTION(BlueprintPure, Category = "Performance Subsystem")
	const FString GetGPUName() const;

	UFUNCTION(BlueprintPure, Category = "Performance Subsystem")
	const FString GetOSVersion() const;

	UFUNCTION(BlueprintPure, Category = "Performance Subsystem")
	const FString GetUsedPhysicalMemory() const;

	UFUNCTION(BlueprintPure, Category = "Performance Subsystem")
	const FString GetAvailablePhysicalMemory() const;

	UFUNCTION(BlueprintPure, Category = "Performance Subsystem")
	const FString GetAvailableVirtualMemory() const;

	UFUNCTION(BlueprintPure, Category = "Performance Subsystem")
	const FString GetUsedVirtualMemory() const;

	UFUNCTION(BlueprintPure, Category = "Performance Subsystem")
	const FString GetPeakUsedVirtualMemory() const;

	UFUNCTION(BlueprintPure, Category = "Performance Subsystem")
	const FString GetPeakUsedPhysicalMemory() const;

public:
	UFUNCTION(BlueprintCallable)
	void Login();

private:
	void OnWebSocketConnected();
	void OnWebSocketError(const FString& Error);
	void OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean);
	void OnWebSocketMessage(const FString& Message);


public:
	void SetSessionId(FString SessionID);
	void SetAccessTokken(FString Payload);
	void SetIsUserLoggedIn(bool LoginStatus);

private:
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

private:
	const class UPOGREndpointSettings* POGRSettings;
	FString SessionId = FString();
	FString AccessTokken = FString();
	UJsonRequestObject* JsonObject;
	TSharedPtr<IWebSocket> WebSocket;
	bool IsLoggedIn;

public:
	UPROPERTY(BlueprintAssignable)
	FOnLoginComplete OnLoginComplete;

	UPROPERTY(BlueprintAssignable)
	FOnSessionCreationCallback OnSessionCreationCallback;
};

/*
	* Write Pure Functions for the Performance Subsystem
	* Make a function that accept URL and Header Content
	* Write a Wrapper Class for the JsonObject that has to be sent over the Http Request
	* Make a Delegate that fires on Backend Response Received
	* Write Async methods for the POSTMAN API's 
*/