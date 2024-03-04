#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "Subsystems/EngineSubsystem.h"
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
class UPOGRSubsystem : public UEngineSubsystem
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
	void CreateSession(const FString& ClientId, const FString& BuildId, const FString& AssociationId);

	UFUNCTION(BlueprintCallable, Category = "POGR Subsystem")
	void DestroySession(const FString& SessionID);

public:
	UFUNCTION(BlueprintCallable, Category = "POGR Subsystem")
	void SendGameMetricsEvent(const FGameMetrics& GameMertrics, const FString& SessionId);

	UFUNCTION(BlueprintCallable, Category = "POGR Subsystem")
	void SendGameUserEvent(const FGameEvent& GameEvent, const FString& SessionId);

	UFUNCTION(BlueprintCallable, Category = "POGR Subsystem")
	void SendGameDataEvent(const UJsonRequestObject* jsonObject, const FString& SessionId);

	UFUNCTION(BlueprintCallable, Category = "POGR Subsystem")
	void SendGameLogsEvent(const FGameLog& GameLog, const FString& SessionId);

	UFUNCTION(BlueprintCallable, Category = "POGR Subsystem")
	void SendGamePerformanceEvent(const FGameSystemMetrics& GamePerformanceMonitor, const FString& SessionId);

public:
	UFUNCTION(BlueprintCallable, Category = "POGR Subsystem | Game Events")
	void SetGameEventAttributes(
		const FString& Event, const FString& SubEvent,
		const FString& EventType, const FString& EventFlag,
		const FString& EventKey, const FString& PlayerId,
		const FString& AchivementName, FGameEvent& GameEvent
	);

	UFUNCTION(BlueprintCallable, Category = "POGR Subsystem | Game Events")
	void SetGameLogsAttributes(
		const FString& Service, const FString& Environment,
		const FString& Severity, const FString& Type,
		const FString& Log, const FString& User_id,
		const FString& Timestamp, const FString& Ip_address,
		const FString& System, const FString& Action, FGameLog& GameLog
	);

	UFUNCTION(BlueprintCallable, Category = "POGR Subsystem | Game Events")
	void SetGameMetricsAttributes(
		const FString& Service, const FString& Environment,
		const int32& Players_online, const float& Average_latency_ms,
		const float& Server_load_percentage, const FString& Location,
		const FString& Game_mode, FGameMetrics& GameMetrics
	);

	UFUNCTION(BlueprintCallable, Category = "POGR Subsystem | Game Events")
	void SetGameMonitorAttributes(
		const float& CPU_usage, const float& Memory_usage,
		const TArray<FString>& Dlls_loaded, const FString& Graphics_quality,
		FGameSystemMetrics& GamePerformanceMonitor
	);

public:
	UFUNCTION(BlueprintPure, Category = "POGR Subsystem | Utilities")
	const FString GetSessionId() const { return ActiveSessionId; };

	UFUNCTION(BlueprintPure, Category = "POGR Subsystem | Utilities")
	const FString GetAccessTokken() const { return AccessTokken; };

	UFUNCTION(BlueprintPure, Category = "POGR Subsystem | Utilities")
	const bool GetIsUserLoggedIn() const { return IsLoggedIn; }

	UFUNCTION(BlueprintPure, Category = "POGR Subsystem | Utilities")
	UJsonRequestObject* GetJsonRequestObject();

	UFUNCTION(BlueprintPure, Category = "POGR Subsystem | Utilities")
	const bool IsSessionActive() const { return bIsSessionActive; }

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	void SendHttpRequest(const FString& URL, const UJsonRequestObject* jsonObject, const FString& SessionId);

private:
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

private:
	void SetSessionId(FString SessionId);
	void SetAccessTokken(FString Payload);
	void SetIsUserLoggedIn(bool LoginStatus);
/*
    * Login Callback Helper Function *
*/
private:
	void OnWebSocketConnected();
	void OnWebSocketError(const FString& Error);
	void OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean);
	void OnWebSocketMessage(const FString& Message);

private:
	const class UPOGREndpointSettings* POGRSettings;
	FString ActiveSessionId = FString();
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

UCLASS()
class UPOGRUISubsystem : public UEditorSubsystem
{
	GENERATED_BODY()
/*
	* POGR UI Helper Function *
*/
#if WITH_EDITOR
public:
	UFUNCTION(BlueprintCallable, Category = "POGR UI Subsystem")
	void SetTabId(FName TabId);

private:
	UFUNCTION(BlueprintCallable, Category = "POGR UI Subsystem")
	const FName GetTabId() const { return WidgetTabId; }

private:
	FName WidgetTabId;
#endif
};