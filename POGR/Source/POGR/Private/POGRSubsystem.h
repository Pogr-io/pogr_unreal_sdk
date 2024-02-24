#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "POGRSubsystem.generated.h"

class IWebSocket;
class UJsonRequestObject;
class UPOGREndpointSettings;
class IHttpRequest;
class IHttpResponse;

typedef TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> FHttpRequestPtr;
typedef TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> FHttpResponsePtr;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoginComplete, bool, bLoginStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSessionCreationCallback);

UCLASS()
class UPOGRSubsystem : public UEditorSubsystem
{
    GENERATED_BODY()

/*
    * POGR Subsystem Intake Helper Function *
*/
public:
	UFUNCTION(BlueprintPure, Category = "POGR User Helper Function")
	const FString GenerateUniqueUserAssociationId() const;

public:
	UFUNCTION(BlueprintCallable, Category = "POGR Subsystem | Login")
	void Login();

	UFUNCTION(BlueprintCallable, Category = "POGR Subsystem")
	void SendGameMetricsEvent(const UJsonRequestObject* jsonObject, const FString& SessionID);

	UFUNCTION(BlueprintCallable, Category = "POGR Subsystem")
	void CreateSession(const FString& ClientId, const FString& BuildId, const FString& AssociationId);

	UFUNCTION(BlueprintCallable, Category = "POGR Subsystem")
	void DestroySession(const FString& SessionID);

	UFUNCTION(BlueprintCallable, Category = "POGR Subsystem")
	void SendGameUserEvent(const UJsonRequestObject* jsonObject, const FString& SessionId);

	UFUNCTION(BlueprintCallable, Category = "POGR Subsystem")
	void SendGameDataEvent(const UJsonRequestObject* jsonObject, const FString& SessionId);

	UFUNCTION(BlueprintCallable, Category = "POGR Subsystem")
	void SendGameLogsEvent(const UJsonRequestObject* jsonObject, const FString& SessionId);

	UFUNCTION(BlueprintCallable, Category = "POGR Subsystem")
	void SendGamePerformanceEvent(const UJsonRequestObject* jsonObject, const FString& SessionId);

public:
	UFUNCTION(BlueprintPure, Category = "POGR Subsystem | Utilities")
	const FString GetSessionId() const { return SessionId; };

	UFUNCTION(BlueprintPure, Category = "POGR Subsystem | Utilities")
	const FString GetAccessTokken() const { return AccessTokken; };

	UFUNCTION(BlueprintPure, Category = "POGR Subsystem | Utilities")
	const bool GetIsUserLoggedIn() const { return IsLoggedIn; }

	UFUNCTION(BlueprintPure, Category = "POGR Subsystem | Utilities")
	UJsonRequestObject* GetJsonRequestObject();

	UFUNCTION(BlueprintPure, Category = "POGR Subsystem | Utilities")
	const bool IsSessionActive() const { return bIsSessionActive; }

private:
	void SendHttpRequest(const FString& URL, const UJsonRequestObject* jsonObject, const FString& SessionID);

private:
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

private:
	void SetSessionId(FString SessionID);
	void SetAccessTokken(FString Payload);
	void SetIsUserLoggedIn(bool LoginStatus);
/*
    * Login Callback Helper Function
*/
private:
	void OnWebSocketConnected();
	void OnWebSocketError(const FString& Error);
	void OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean);
	void OnWebSocketMessage(const FString& Message);
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

private:
	const class UPOGREndpointSettings* POGRSettings;
	FString SessionId = FString();
	FString AccessTokken = FString();
	UJsonRequestObject* JsonObject;
	TSharedPtr<IWebSocket> WebSocket;
	bool IsLoggedIn;
	bool bIsSessionActive;

public:
	UPROPERTY(BlueprintAssignable)
	FOnLoginComplete OnLoginComplete;

	UPROPERTY(BlueprintAssignable)
	FOnSessionCreationCallback OnSessionCreationCallback;
};