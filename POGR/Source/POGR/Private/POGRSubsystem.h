#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "Helpers/POGRURLFilters.h"
#include "POGRSubsystem.generated.h"

class IWebSocket;
class UJsonRequestObject;
class UPOGREndpointSettings;
class IHttpRequest;
class IHttpResponse;
class UTexture2DDynamic;

typedef TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> FHttpRequestPtr;
typedef TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> FHttpResponsePtr;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoginComplete, bool, bLoginStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSessionCreationCallback);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOrganizationCallback);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameCallback);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPayloadDataUpdate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPayloadCreationCallback);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPayloadCallback, FString, Content);

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

	UFUNCTION(BlueprintCallable, Category = "POGR Subsystem | LogOut")
	void LogOut();

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
	UFUNCTION(BlueprintCallable)
	void GetOrganizationData();

	UFUNCTION(BlueprintCallable)
	void GetOrganizationGameData(const FString& GameUUID);

	UFUNCTION(BlueprintCallable)
	void GetUserProfileData();

public:
	UFUNCTION(BlueprintCallable)
	void ListDataPayloads();

	UFUNCTION(BlueprintCallable)
	void UpdateDataStatus(EAcceptedStatus AcceptedStatus, FString DataId);

	UFUNCTION(BlueprintCallable)
	void GetDataPayloadDefinition(FString DataId);

	UFUNCTION(BlueprintCallable)
	void GetDataReceived(FString DataId);
	
	UFUNCTION(BlueprintCallable)
	FString GetPogrUrl(URLAction Action, URLDefinition Definition, EAcceptedStatus Status = EAcceptedStatus::Ignored, FString BuildId = FString("None"), FString DataId = FString("None"));

	UFUNCTION(BlueprintPure)
	const TArray<FDataPayload> GetDataPayloadArray() const { return DataPayloads; }

	UFUNCTION(BlueprintPure)
	const TArray<FOrganizationData> GetOrganizationDataArray() const { return OrganizationData; }

	UFUNCTION(BlueprintPure)
	const TArray<FOrganizationGameData> GetOrganizationGameDataArray() const { return OrganizationGameData; }

	UFUNCTION(BlueprintCallable)
	void SetPayloadDatatype(URLDefinition PayloadDatatype);

	UFUNCTION(BlueprintCallable)
	FString ConstructOrgGameURL(const FString& Id);

	UFUNCTION(BlueprintCallable)
	FString ConstructGameDetailURL(const FString& Id);

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

	UFUNCTION(BlueprintPure, Category = "POGR Subsystem | Utilities")
	const FString GetGameBuildId() const { return GameBuildId; }

	UFUNCTION(BlueprintPure, Category = "POGR Subsystem | Utilities")
	const FString GetContentAsString() const { return ContentAsString; }

	UFUNCTION(BlueprintPure, Category = "POGR Subsystem | Utilities")
	const FString GetSelectedOptionValue() const;

	// Selecting An Organization And Game
public:
	UFUNCTION(BlueprintCallable)
	void SetOrganizationOption(FString OrganizationValue);

	UFUNCTION(BlueprintPure)
	const FOrganizationData GetOrganization() const { return Organization; }

	UFUNCTION(BlueprintCallable)
	void SetGameOption(FString GameValue);

	UFUNCTION(BlueprintPure)
	const FOrganizationGameData GetSelectedGameTitle() const { return Game; }

	UFUNCTION(BlueprintPure)
	const FUserProfileData GetUserProfile() const { return UserProfileData; }

public:
	UFUNCTION(BlueprintCallable)
	void SetGameTitleTexture(UTexture2DDynamic* Texture);

	UFUNCTION(BlueprintCallable)
	void SetOrganizationTitleTexture(UTexture2DDynamic* Texture);

	UFUNCTION(BlueprintCallable)
	void SetUserProfileTexture(UTexture2DDynamic* Texture);

	UFUNCTION(BlueprintPure)
	const UTexture2DDynamic* GetGameTitleTexture() const { return GameTitleTexture; }

	UFUNCTION(BlueprintPure)
	const UTexture2DDynamic* GetOrganizationTitleTexture() const { return OrganizationTitleTexture; }

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
	void SetGameBuildId(FString GameUUID);
	void SetContentAsString(FString Content);
	const URLDefinition GetPayloadDatatype() const { return PayloadDefinition; }
	const bool GetUpdateOptions() const { return bUpdateOptions; }
/*
    * Login Callback Helper Function *
*/
private:
	void OnWebSocketConnected();
	void OnWebSocketError(const FString& Error);
	void OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean);
	void OnWebSocketMessage(const FString& Message);

public:
	UPROPERTY(BlueprintAssignable)
	FOnLoginComplete OnLoginComplete;

	UPROPERTY(BlueprintAssignable)
	FOnSessionCreationCallback OnSessionCreationCallback;

	UPROPERTY(BlueprintAssignable)
	FOnPayloadCreationCallback OnPayloadCreationCallback;

	UPROPERTY(BlueprintAssignable)
	FOnPayloadCallback OnPayloadCallback;

	UPROPERTY(BlueprintAssignable)
	FOnPayloadDataUpdate OnPayloadDataUpdate;

	UPROPERTY(BlueprintAssignable)
	FOnOrganizationCallback OnOrganizationCallback;

	UPROPERTY(BlueprintAssignable)
	FOnGameCallback OnGameCallback;

private:
	const class UPOGREndpointSettings* POGRSettings;
	FString ActiveSessionId = FString();
	FString AccessTokken = FString();
	UJsonRequestObject* JsonObject;
	TSharedPtr<IWebSocket> WebSocket;
	bool IsLoggedIn;
	bool bIsSessionActive;
	FString GameBuildId;

private:
	FUserProfileData UserProfileData;
	TArray<FOrganizationData> OrganizationData;
	TArray<FOrganizationGameData> OrganizationGameData;
	TArray<FDataPayload> DataPayloads;
	FString ContentAsString;

private:
	UTexture2DDynamic* GameTitleTexture;
	UTexture2DDynamic* OrganizationTitleTexture;

private:
	URLDefinition PayloadDefinition = URLDefinition::Data;

private:
	FOrganizationData Organization;
	FOrganizationGameData Game;
	
private:
	bool bUpdateOptions = true;	
};