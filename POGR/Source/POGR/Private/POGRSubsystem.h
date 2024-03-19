#pragma once

#include "CoreMinimal.h"
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

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPayloadDataUpdate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPayloadCreationCallback);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPayloadCallback, FString, Content);

UENUM(BlueprintType)
enum EAcceptedStatus : uint8
{
	Pending,
	Saved,
	Ignored,
	None
};

USTRUCT(BlueprintType)
struct FOrganizationData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString UUID;

	UPROPERTY(BlueprintReadOnly)
	FString Name;
	FString CreatedOn;
	FString Type;

	UPROPERTY(BlueprintReadOnly)
	FString URL;
};

USTRUCT(BlueprintType)
struct FOrganizationGameData
{
	GENERATED_BODY()

	FString UUID;
	FString StudioUUID;
	FString GameTitle;
	FString URL;
	FString CreatedOn;
};

USTRUCT(BlueprintType)
struct FUserProfileData
{
	GENERATED_BODY()

	FString UserName;
	FString DisplayName;
	FString AvatarURL;
	int32 Level;
	int32 Exp;
	int32 RequiredExp;
};

USTRUCT(BlueprintType)
struct FDataPayload
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Id;

	UPROPERTY(BlueprintReadWrite)
	FString ClientId;

	UPROPERTY(BlueprintReadWrite)
	FString BuildId;

	UPROPERTY(BlueprintReadWrite)
	TEnumAsByte<EAcceptedStatus> AcceptedStatus;

	UPROPERTY(BlueprintReadWrite)
	FString DataReceivedDate;

	UPROPERTY(BlueprintReadWrite)
	int32 Calls;
};

UENUM()
enum URLDefinition : uint8 {
	Data,
	Events,
	Logs,
	Metrics
};

UENUM()
enum URLAction : uint8 {
	List,
	Update,
	Definition,
	Receiving
};

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
	UFUNCTION(BlueprintCallable)
	void GetOrganizationData(FString POGRClient, FString LoginTokken);

	UFUNCTION(BlueprintCallable)
	void GetOrganizationGameData(FString POGRClient, FString LoginTokken);

	UFUNCTION(BlueprintCallable)
	void GetUserProfileData(FString POGRClient, FString LoginTokken);

public:
	UFUNCTION(BlueprintCallable)
	void ListDataPayloads(FString POGRClient, FString LoginTokken, FString BuildId);

	UFUNCTION(BlueprintCallable)
	void UpdateDataStatus(FString POGRClient, FString LoginTokken, EAcceptedStatus AcceptedStatus, FString BuildId, FString DataId);

	UFUNCTION(BlueprintCallable)
	void GetDataPayloadDefinition(FString POGRClient, FString LoginTokken, FString DataId, FString BuildId);

	UFUNCTION(BlueprintCallable)
	void GetDataReceived(FString POGRClient, FString LoginTokken, FString DataId, FString BuildId);
	
	UFUNCTION(BlueprintCallable)
	FString GetPogrUrl(URLAction Action, URLDefinition Definition, EAcceptedStatus Status = EAcceptedStatus::Ignored, FString BuildId = FString("None"), FString DataId = FString("None"));

	UFUNCTION(BlueprintPure)
	const TArray<FDataPayload> GetDataPayloadArray() const { return DataPayloads; }

	UFUNCTION(BlueprintPure)
	const TArray<FOrganizationData> GetOrganizationDataArray() const { return OrganizationData; }

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

private:
	FUserProfileData UserProfileData;
	TArray<FOrganizationData> OrganizationData;
	TArray<FOrganizationGameData> OrganizationGameData;
	TArray<FDataPayload> DataPayloads;
	FString ContentAsString;

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

public:
	void SetContentAsString(FString Content);

	UFUNCTION(BlueprintPure)
	const FString GetContentAsString() const { return ContentAsString; }

public:
	UFUNCTION(BlueprintCallable)
	void SetPayloadDatatype(URLDefinition PayloadDatatype);

	UFUNCTION(BlueprintPure)
	const FString GetSelectedOptionValue() const;

private:
	const URLDefinition GetPayloadDatatype() const { return PayloadDefinition; }

private:
	URLDefinition PayloadDefinition = URLDefinition::Data;
	
	// Selecting An Organization
public:
	UFUNCTION(BlueprintCallable)
	void SetOrganizationOption(FString OrganizationValue);

	UFUNCTION(BlueprintPure)
	const FString GetOrganizationName() const { return OrganizationName; }

private:
	FString OrganizationName;
};