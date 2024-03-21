#include "POGRSubsystem.h"
#include "POGR.h"
#include "WebSockets/Public/IWebSocket.h"
#include "WebSockets/Public/WebSocketsModule.h"
#include "WebSockets/Public/IWebSocketsManager.h"
#include "JsonUtilities/Public/JsonObjectConverter.h"
#include "Misc/Guid.h"
#include "Http.h"
#include "Json.h"
#include "POGRSettings.h"
#include "POGRJsonObject.h"
#include "Async/TaskGraphInterfaces.h"
#include "Helpers/POGRGameEvent.h"
#include "Helpers/POGRGameLogs.h"
#include "Helpers/POGRGameMetrics.h"
#include "Helpers/POGRGameMonitor.h"

const FString const POGR_CLIENT = FString("5XJ4WBU1Q52PV6RKKNI6EB7AF3RDU2U8HTW2YA0Z2YQIZCGIZ6NLUD52ERPV0IGAXG19WM9ZF3PVBUYGWTRCLIQG75J3P11WRPVOCGXCHZLEN86WL3DDL7GOHIEM7E8G");
const FString const POGR_SESSION = FString("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1aWQiOiI1MTRlYzJkOC03ZTY3LTRkMDQtODcxOS03OTkzYzU0ZTkwZmMiLCJzZXNzaW9uX3Rva2VuIjoib2dwcWxtZHJpc25pM2VjbXNsZTlvajVwcDhkOTExdWNsM3JsYXFodTNmZDhlNTB0cmdyZG1oNGtqY2YxdHUzdXpiZmwwMHh1cjJibmVxMnBiaTI5bnJ6cmc4OWIyOXF3ZWd1Ymphd20zeHg0cTUxamNwYWJ3eXIxejAzOHQwbzc0cWNnMmtpZDJpdXFuOTd6eHZ6MmgzaHpramR3aGQzaXlwZjYwbTNobXExdmkwcTlqcDBmcXhsZW9ndTdlbndpaWpiNGh2YnVucmtneHd6czcwNmdzcmV6cWdyenluODR0am4wd3N1eGt4MG9xeDhjeHJycjZzanVibjJxbGpnbCIsImlhdCI6MTcxMDk2MzQ3N30.w4RKEgyGzCmFRLh7q7jiyjf8N10ylRHMrtAzyFDPlTU");


const FString UPOGRSubsystem::GenerateUniqueUserAssociationId() const
{
    FGuid RandomGuid = FGuid::NewGuid();
    FString AssociationId = FString::Printf(TEXT("%s"), *RandomGuid.ToString());
    return AssociationId;
}

void UPOGRSubsystem::CreateSession(const FString& ClientId, const FString& BuildId, const FString& AssociationId)
{    
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();

    RequestObj->SetStringField("association_id", AssociationId);

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(RequestObj, Writer);


    Request->SetURL(POGRSettings->GetInitEndpoint());
    Request->SetVerb(TEXT("POST"));

    Request->SetHeader("Content-Type", "application/json");
    Request->SetHeader("POGR_CLIENT", ClientId);
    Request->SetHeader("POGR_BUILD", BuildId);
    
    Request->SetContentAsString(RequestBody);

    Request->OnProcessRequestComplete().BindLambda(
        [this](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccess)
        {
            if (bSuccess && HttpResponse.IsValid() && HttpResponse->GetResponseCode() == EHttpResponseCodes::Type::Ok)
            {
                TSharedPtr<FJsonObject> ResponseObj;
                if (HttpResponse != NULL) {
                    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
                    FJsonSerializer::Deserialize(Reader, ResponseObj);
                }

                if (ResponseObj->HasField("payload"))
                {
                    // Get the "payload" object
                    TSharedPtr<FJsonObject> PayloadObject = ResponseObj->GetObjectField("payload");

                    // Check if the "payload" object contains the "redirect_url" field
                    if (PayloadObject->HasField("session_id"))
                    {
                        FString SessionId = PayloadObject->GetStringField("session_id");

                        if (!SessionId.IsEmpty())
                        {
                            SetSessionId(SessionId);
                            bIsSessionActive = true;
                            OnSessionCreationCallback.Broadcast();
                        }
                    }
                }
            }
            else
                UE_LOG(LogTemp, Error, TEXT("Failed to Create Session"));
        }
    );

    Request->ProcessRequest();
}

void UPOGRSubsystem::DestroySession(const FString& SessionId)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(POGRSettings->GetShutdownEndpoint());
    Request->SetVerb(TEXT("POST"));

    Request->SetHeader("Content-Type", "application/json");
    Request->SetHeader("INTAKE_SESSION_ID", SessionId);

    Request->OnProcessRequestComplete().BindLambda(
        [this](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccess)
        {
            if (bSuccess && HttpResponse.IsValid() && HttpResponse->GetResponseCode() == EHttpResponseCodes::Type::Ok)
            {
                bIsSessionActive = false;
            }
        }
    );

    Request->ProcessRequest();
}

void UPOGRSubsystem::SendGameMetricsEvent(const FGameMetrics& GameMertrics, const FString& SessionId)
{
    static FCriticalSection Mutex;

    FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([=]()
    {
       FScopeLock Lock(&Mutex);
       UJsonRequestObject* JsonObject = this->GetJsonRequestObject();
       JsonObject->SetStringField(FString("service"), GameMertrics.service);
       JsonObject->SetStringField(FString("environment"), GameMertrics.environment);

       UJsonRequestObject* MetricsData = this->GetJsonRequestObject();
       MetricsData->SetNumberField(FString("players_online"), GameMertrics.metrics.players_online);
       MetricsData->SetNumberField(FString("average_latency_ms"), GameMertrics.metrics.average_latency_ms);
       MetricsData->SetNumberField(FString("server_load_percentage"), GameMertrics.metrics.server_load_percentage);
       JsonObject->GetJsonRequestObject()->SetObjectField(FString("metrics"), MetricsData->GetJsonRequestObject());

       UJsonRequestObject* TagsData = this->GetJsonRequestObject();
       TagsData->SetStringField(FString("location"), GameMertrics.tags.location);
       TagsData->SetStringField(FString("game_mode"), GameMertrics.tags.game_mode);
       JsonObject->GetJsonRequestObject()->SetObjectField(FString("tags"), TagsData->GetJsonRequestObject());

       SendHttpRequest(POGRSettings->GetMetricsEndpoint(), JsonObject, SessionId);
    }, TStatId(), nullptr, ENamedThreads::AnyThread);
}

void UPOGRSubsystem::SendGameUserEvent(const FGameEvent& GameEvent, const FString& SessionId)
{
    static FCriticalSection Mutex;

    FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([=]()
    {
       FScopeLock Lock(&Mutex);
       UJsonRequestObject* JsonObject = this->GetJsonRequestObject();
       JsonObject->SetStringField("event", GameEvent.event);
       JsonObject->SetStringField("sub_event", GameEvent.sub_event);
       JsonObject->SetStringField("event_type", GameEvent.event_type);
       JsonObject->SetStringField("event_flag", GameEvent.event_flag);
       JsonObject->SetStringField("event_key", GameEvent.event_key);

       UJsonRequestObject* EventData = this->GetJsonRequestObject();
       EventData->SetStringField("player_id", GameEvent.event_data.player_id);
       EventData->SetStringField("achievement_name", GameEvent.event_data.achievement_name);
       JsonObject->GetJsonRequestObject()->SetObjectField("event_data", EventData->GetJsonRequestObject());

       SendHttpRequest(FString(POGRSettings->GetEventEndpoint()), JsonObject, SessionId);
    }, TStatId(), nullptr, ENamedThreads::AnyThread);
}

void UPOGRSubsystem::SendGameDataEvent(const UJsonRequestObject* jsonObject, const FString& SessionId)
{
    FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([=]()
    {
       SendHttpRequest(POGRSettings->GetDataEndpoint(), jsonObject, SessionId);
    }, TStatId(), nullptr, ENamedThreads::AnyThread);
}

void UPOGRSubsystem::SendGameLogsEvent(const FGameLog& GameLog, const FString& SessionId)
{
    static FCriticalSection Mutex;

    FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([=]()
        {
            FScopeLock Lock(&Mutex);
            UJsonRequestObject* JsonObject = this->GetJsonRequestObject();
            JsonObject->SetStringField(FString("service"), GameLog.service);
            JsonObject->SetStringField(FString("environment"), GameLog.environment);
            JsonObject->SetStringField(FString("severity"), GameLog.severity);
            JsonObject->SetStringField(FString("type"), GameLog.type);
            JsonObject->SetStringField(FString("log"), GameLog.log);

            UJsonRequestObject* DataObject = this->GetJsonRequestObject();
            DataObject->SetStringField(FString("user_id"), GameLog.data.user_id);
            DataObject->SetStringField(FString("timestamp"), GameLog.data.timestamp);
            DataObject->SetStringField(FString("ip_address"), GameLog.data.ip_address);
            JsonObject->GetJsonRequestObject()->SetObjectField(FString("Data"), DataObject->GetJsonRequestObject());

            UJsonRequestObject* TagsObject = this->GetJsonRequestObject();
            TagsObject->SetStringField(FString("system"), GameLog.tags.system);
            TagsObject->SetStringField(FString("action"), GameLog.tags.action);
            JsonObject->GetJsonRequestObject()->SetObjectField(FString("tags"), TagsObject->GetJsonRequestObject());

            SendHttpRequest(POGRSettings->GetLogsEndpoint(), JsonObject, SessionId);
    }, TStatId(), nullptr, ENamedThreads::AnyThread);
}

void UPOGRSubsystem::SendGamePerformanceEvent(const FGameSystemMetrics& GamePerformanceMonitor, const FString& SessionId)
{
    static FCriticalSection Mutex;

    FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([=]()
    {
       FScopeLock Lock(&Mutex);
       UJsonRequestObject* JsonObject = this->GetJsonRequestObject();
       JsonObject->SetNumberField(FString("cpu_usage"), GamePerformanceMonitor.cpu_usage);
       JsonObject->SetNumberField(FString("memory_usage"), GamePerformanceMonitor.memory_usage);
       JsonObject->SetArrayField(FString("dlls_loaded"), GamePerformanceMonitor.dlls_loaded);

       UJsonRequestObject* SettingsObject = this->GetJsonRequestObject();
       SettingsObject->SetStringField(FString("graphics_quality"), GamePerformanceMonitor.settings.graphics_quality);
       JsonObject->GetJsonRequestObject()->SetObjectField(FString("Data"), SettingsObject->GetJsonRequestObject());

       SendHttpRequest(POGRSettings->GetMonitorEndpoint(), JsonObject, SessionId);
    }, TStatId(), nullptr, ENamedThreads::AnyThread);
}

void UPOGRSubsystem::SetGameEventAttributes(
    const FString& Event, const FString& SubEvent,
    const FString& EventType, const FString& EventFlag,
    const FString& EventKey, const FString& PlayerId,
    const FString& AchivementName, FGameEvent& GameEvent)
{
    FGameEvent GameEventData;
    GameEventData.event = Event;
    GameEventData.sub_event = SubEvent;
    GameEventData.event_type = EventType;
    GameEventData.event_flag = EventFlag;
    GameEventData.event_key = EventKey;
    GameEventData.event_data.player_id = PlayerId;
    GameEventData.event_data.achievement_name = AchivementName;

    GameEvent = GameEventData;
}

void UPOGRSubsystem::SetGameLogsAttributes(
    const FString& Service, const FString& Environment,
    const FString& Severity, const FString& Type,
    const FString& Log, const FString& User_id,
    const FString& Timestamp, const FString& Ip_address,
    const FString& System, const FString& Action, FGameLog& GameLog)
{
    FGameLog GameLogData;
    GameLogData.service = Service;
    GameLogData.environment = Environment;
    GameLogData.severity = Severity;
    GameLogData.type = Type;
    GameLogData.log = Log;
    GameLogData.data.user_id = User_id;
    GameLogData.data.timestamp = Timestamp;
    GameLogData.data.ip_address = Ip_address;
    GameLogData.tags.system = System;
    GameLogData.tags.action = Action;

    GameLog = GameLogData;
}

void UPOGRSubsystem::SetGameMetricsAttributes(
    const FString& Service, const FString& Environment,
    const int32& Players_online, const float& Average_latency_ms,
    const float& Server_load_percentage, const FString& Location,
    const FString& Game_mode, FGameMetrics& GameMetrics)
{
    FGameMetrics GameMetricsData;
    GameMetricsData.service = Service;
    GameMetricsData.environment = Environment;
    GameMetricsData.metrics.players_online = Players_online;
    GameMetricsData.metrics.average_latency_ms = Average_latency_ms;
    GameMetricsData.metrics.server_load_percentage = Server_load_percentage;
    GameMetricsData.tags.location = Location;
    GameMetricsData.tags.game_mode = Game_mode;

    GameMetrics = GameMetricsData;
}

void UPOGRSubsystem::SetGameMonitorAttributes(
    const float& CPU_usage, const float& Memory_usage,
    const TArray<FString>& Dlls_loaded, const FString& Graphics_quality,
    FGameSystemMetrics& GamePerformanceMonitor)
{
    FGameSystemMetrics GameSystemMetrics;
    GameSystemMetrics.cpu_usage = CPU_usage;
    GameSystemMetrics.memory_usage = Memory_usage;
    GameSystemMetrics.dlls_loaded = Dlls_loaded;
    GameSystemMetrics.settings.graphics_quality = Graphics_quality;

    GamePerformanceMonitor = GameSystemMetrics;
}

void UPOGRSubsystem::GetOrganizationData(FString POGRClient, FString LoginTokken)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(FString("https://api-stage.pogr.io/v1/organizations/plugins/studios"));
    Request->SetVerb(TEXT("GET"));

    Request->SetHeader("Content-Type", "application/json");
    Request->SetHeader("POGR_CLIENT", POGRClient);
    Request->SetHeader("POGR_SESSION", POGR_SESSION);

    Request->OnProcessRequestComplete().BindLambda(
        [this](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccess)
        {
            if (bSuccess && HttpResponse.IsValid() && HttpResponse->GetResponseCode() == EHttpResponseCodes::Type::Ok)
            {
                TSharedPtr<FJsonObject> ResponseObj;
                if (HttpResponse != NULL) {
                    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
                    FJsonSerializer::Deserialize(Reader, ResponseObj);
                }

                if (ResponseObj->HasField("payload"))
                {
                    // Get the "payload" object
                    TSharedPtr<FJsonObject> PayloadObject = ResponseObj->GetObjectField("payload");

                    // Check if the "payload" object contains the "organizations" field
                    const TArray<TSharedPtr<FJsonValue>>* OrganizationsArray;
                    if (PayloadObject->TryGetArrayField("organizations", OrganizationsArray))
                    {
                        for (const auto& OrganizationValue : *OrganizationsArray)
                        {
                            const TSharedPtr<FJsonObject> OrganizationObject = OrganizationValue->AsObject();
                            if (OrganizationObject.IsValid())
                            {
                                FOrganizationData Organization;

                                Organization.UUID = OrganizationObject->GetStringField("uid");
                                Organization.Name = OrganizationObject->GetStringField("name");
                                Organization.CreatedOn = OrganizationObject->GetStringField("created_on");
                                Organization.Type = OrganizationObject->GetStringField("type");

                                const TSharedPtr<FJsonObject> LogoObject = OrganizationObject->GetObjectField("logo");
                                if (LogoObject.IsValid())
                                {
                                    Organization.URL = LogoObject->GetStringField("url");
                                }

                                OrganizationData.Add(Organization);
                                OnOrganizationCallback.Broadcast();
                            }
                        }
                    }
                }
            }
            else
                UE_LOG(LogTemp, Error, TEXT("Failed to Fetch Oragnization Data"));
        }
    );

}

void UPOGRSubsystem::GetOrganizationGameData(FString POGRClient, FString LoginTokken, const FString& GameUUID)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(ConstructOrgGameURL(GameUUID));
    Request->SetVerb(TEXT("GET"));

    Request->SetHeader("Content-Type", "application/json");
    Request->SetHeader("POGR_CLIENT", POGRClient);
    Request->SetHeader("POGR_SESSION", POGR_SESSION);

    Request->OnProcessRequestComplete().BindLambda(
        [this](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccess)
        {
            if (bSuccess && HttpResponse.IsValid() && HttpResponse->GetResponseCode() == EHttpResponseCodes::Type::Ok)
            {
                TSharedPtr<FJsonObject> ResponseObj;
                if (HttpResponse != NULL) {
                    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
                    FJsonSerializer::Deserialize(Reader, ResponseObj);
                }

                const TArray<TSharedPtr<FJsonValue>>* PayloadArray;
                if (ResponseObj->TryGetArrayField("payload", PayloadArray))
                {
                    for (const auto& PayloadValue : *PayloadArray)
                    {
                        const TSharedPtr<FJsonObject> PayloadObject = PayloadValue->AsObject();
                        if (PayloadObject.IsValid())
                        {
                            FOrganizationGameData Organization;

                            Organization.UUID = PayloadObject->GetStringField("uid");
                            Organization.StudioUUID = PayloadObject->GetStringField("studio_org_uid");
                            Organization.GameTitle = PayloadObject->GetStringField("name");
                            Organization.CreatedOn = PayloadObject->GetStringField("created_on");

                            const TSharedPtr<FJsonObject> LogoObject = PayloadObject->GetObjectField("logo");
                            if (LogoObject.IsValid())
                            {
                                Organization.URL = LogoObject->GetStringField("url");
                            }

                            OrganizationGameData.Add(Organization);
                            OnGameCallback.Broadcast();
                        }
                    }
                }
            }
            else
                UE_LOG(LogTemp, Error, TEXT("Failed to Fetch Oragnization Game Data"));
        }
    );

    Request->ProcessRequest();
}

void UPOGRSubsystem::GetUserProfileData(FString POGRClient, FString LoginTokken)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(FString("https://api-stage.pogr.io/v1/user/plugins/profile"));
    Request->SetVerb(TEXT("GET"));

    Request->SetHeader("Content-Type", "application/json");
    Request->SetHeader("POGR_CLIENT", POGRClient);
    Request->SetHeader("POGR_SESSION", LoginTokken);

    Request->OnProcessRequestComplete().BindLambda(
        [this](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccess)
        {
            if (bSuccess && HttpResponse.IsValid() && HttpResponse->GetResponseCode() == EHttpResponseCodes::Type::Ok)
            {
                TSharedPtr<FJsonObject> ResponseObj;
                if (HttpResponse != NULL) {
                    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
                    FJsonSerializer::Deserialize(Reader, ResponseObj);
                }

                if (ResponseObj->HasField("payload"))
                {
                    // Get the "payload" object
                    TSharedPtr<FJsonObject> PayloadObject = ResponseObj->GetObjectField("payload");

                    UserProfileData.UserName = PayloadObject->GetStringField("username");
                    UserProfileData.DisplayName = PayloadObject->GetStringField("display_name");
                    UserProfileData.AvatarURL = PayloadObject->GetStringField("avatar");
                    UserProfileData.Level = FCString::Atoi(*PayloadObject->GetStringField("level"));
                    UserProfileData.Exp = FCString::Atoi(*PayloadObject->GetStringField("total_exp"));
                    UserProfileData.RequiredExp = FCString::Atoi(*PayloadObject->GetStringField("exp_required"));
                }
            }
            else
                UE_LOG(LogTemp, Error, TEXT("Failed to Fetch User Profile Data"));
        }
    );

    Request->ProcessRequest();
}

void UPOGRSubsystem::ListDataPayloads(FString POGRClient, FString LoginTokken, FString BuildId)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(GetPogrUrl(URLAction::List, GetPayloadDatatype(), EAcceptedStatus::Ignored, BuildId));
    Request->SetVerb(TEXT("GET"));

    Request->SetHeader("Content-Type", "application/json");
    Request->SetHeader("POGR_CLIENT", POGRClient);
    Request->SetHeader("POGR_SESSION", LoginTokken);

    Request->OnProcessRequestComplete().BindLambda(
        [this](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccess)
        {
            if (bSuccess && HttpResponse.IsValid() && HttpResponse->GetResponseCode() == EHttpResponseCodes::Type::Ok)
            {
                TSharedPtr<FJsonObject> ResponseObj;
                if (HttpResponse != NULL) {
                    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
                    FJsonSerializer::Deserialize(Reader, ResponseObj);
                }
                
                if (ResponseObj->HasField("payload"))
                {
                    // Get the "payload" object
                    TSharedPtr<FJsonObject> PayloadObject = ResponseObj->GetObjectField("payload");

                    // Check if the "payload" object contains the "organizations" field
                    const TArray<TSharedPtr<FJsonValue>>* DataArray;
                    if (PayloadObject->TryGetArrayField("data", DataArray))
                    {
                        DataPayloads.Empty();

                        for (const auto& DataValue : *DataArray)
                        {
                            const TSharedPtr<FJsonObject> DataObject = DataValue->AsObject();
                            if (DataObject.IsValid())
                            {
                                FDataPayload DataPayload;

                                DataPayload.Id = DataObject->GetStringField("id");
                                DataPayload.ClientId = DataObject->GetStringField("client_id");
                                DataPayload.BuildId = DataObject->GetStringField("build_id");
                                DataPayload.AcceptedStatus = (EAcceptedStatus)FCString::Atoi(*DataObject->GetStringField("accepted_status"));
                                DataPayload.DataReceivedDate = DataObject->GetStringField("first_data_received_date");
                                DataPayload.Calls = FCString::Atoi(*DataObject->GetStringField("calls"));

                                DataPayloads.Add(DataPayload);

                                OnPayloadCreationCallback.Broadcast();
                            }
                        }
                    }
                    else {
                        DataPayloads.Empty();
                        OnPayloadCreationCallback.Broadcast();
                    }
                }
            }
            else
                UE_LOG(LogTemp, Error, TEXT("Failed to Payloads List Data"));
        }
    );

    Request->ProcessRequest();
}

void UPOGRSubsystem::UpdateDataStatus(FString POGRClient, FString LoginTokken, EAcceptedStatus AcceptedStatus, FString BuildId, FString DataId)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(GetPogrUrl(URLAction::Update, GetPayloadDatatype(), AcceptedStatus, BuildId, DataId));
    Request->SetVerb(TEXT("PUT"));

    Request->SetHeader("Content-Type", "application/json");
    Request->SetHeader("POGR_CLIENT", POGRClient);
    Request->SetHeader("POGR_SESSION", LoginTokken);

    Request->OnProcessRequestComplete().BindLambda(
        [this](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccess)
        {
            if (bSuccess && HttpResponse.IsValid() && HttpResponse->GetResponseCode() == EHttpResponseCodes::Type::Ok)
            {
                OnPayloadDataUpdate.Broadcast();
                UE_LOG(LogTemp, Warning, TEXT("%s"), *HttpResponse->GetContentAsString());
            }
            else
                UE_LOG(LogTemp, Error, TEXT("Failed to Update Payload Data Status"));
        }
    );

    Request->ProcessRequest();
}

void UPOGRSubsystem::GetDataPayloadDefinition(FString POGRClient, FString LoginTokken, FString DataId, FString BuildId)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(GetPogrUrl(URLAction::Definition, GetPayloadDatatype(), EAcceptedStatus::None, BuildId, DataId));
    Request->SetVerb(TEXT("GET"));

    Request->SetHeader("Content-Type", "application/json");
    Request->SetHeader("POGR_CLIENT", POGRClient);
    Request->SetHeader("POGR_SESSION", LoginTokken);

    Request->OnProcessRequestComplete().BindLambda(
        [this](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccess)
        {
            if (bSuccess && HttpResponse.IsValid() && HttpResponse->GetResponseCode() == EHttpResponseCodes::Type::Ok)
            {
                TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

                TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
                if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
                {
                    // Serialize the JSON object back to a formatted string
                    FString JsonString;
                    TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonString);
                    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);
                    SetContentAsString(JsonString);
                }
            }
            else
                UE_LOG(LogTemp, Error, TEXT("Failed to Payloads List Data"));
        }
    );

    Request->ProcessRequest();
}

void UPOGRSubsystem::GetDataReceived(FString POGRClient, FString LoginTokken, FString DataId, FString BuildId)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(GetPogrUrl(URLAction::Receiving, GetPayloadDatatype(), EAcceptedStatus::None, BuildId, DataId));
    Request->SetVerb(TEXT("GET"));

    Request->SetHeader("Content-Type", "application/json");
    Request->SetHeader("POGR_CLIENT", POGRClient);
    Request->SetHeader("POGR_SESSION", LoginTokken);

    Request->OnProcessRequestComplete().BindLambda(
        [this](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccess)
        {
            if (bSuccess && HttpResponse.IsValid() && HttpResponse->GetResponseCode() == EHttpResponseCodes::Type::Ok)
            {
                TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

                TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
                if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
                {
                    // Serialize the JSON object back to a formatted string
                    FString JsonString;
                    TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonString);
                    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);
                    SetContentAsString(JsonString);
                }
            }
            else
                UE_LOG(LogTemp, Error, TEXT("Failed to Payloads List Data"));
        }
    );

    Request->ProcessRequest();
}

FString UPOGRSubsystem::GetPogrUrl(URLAction Action, URLDefinition Definition, EAcceptedStatus Status, FString BuildId, FString DataId)
{
    FString URL = FString();
    FString BaseURL = FString("https://api-stage.pogr.io/v1/widget/data/developer-review/");

    switch (Action)
    {
    case URLAction::List:
        switch (Definition)
        {
            case URLDefinition::Data:
                URL = BaseURL + "list-data-payloads?build_id=" + BuildId;
                break;
            case URLDefinition::Events:
                URL = BaseURL + "list-events-payloads?build_id=" + BuildId;
                break;
            case URLDefinition::Metrics:
                URL = BaseURL + "list-metrics-payloads?build_id=" + BuildId;
                break;
            case URLDefinition::Logs:
                URL = BaseURL + "list-logs-payloads?build_id=" + BuildId;
                break;
        }
        break;
    case URLAction::Update:
        switch (Definition)
        {
            case URLDefinition::Data:
                switch (Status)
                {
                    case EAcceptedStatus::Pending:
                        URL = BaseURL + "update-status/data/" + DataId + "?status=Pending&build_id=" + BuildId;
                        break;
                    case EAcceptedStatus::Saved:
                        URL = BaseURL + "update-status/data/" + DataId + "?status=Save&build_id=" + BuildId;
                        break;
                    case EAcceptedStatus::Ignored:
                        URL = BaseURL + "update-status/data/" + DataId + "?status=Ignore&build_id=" + BuildId;
                        break;
                }
                break;
            case URLDefinition::Events:
                switch (Status)
                {
                    case EAcceptedStatus::Pending:
                        URL = BaseURL + "update-status/events/" + DataId + "?status=Pending&build_id=" + BuildId;
                        break;
                    case EAcceptedStatus::Saved:
                        URL = BaseURL + "update-status/events/" + DataId + "?status=Save&build_id=" + BuildId;
                        break;
                    case EAcceptedStatus::Ignored:
                        URL = BaseURL + "update-status/events/" + DataId + "?status=Ignore&build_id=" + BuildId;
                        break;
                }
                break;
            case URLDefinition::Metrics:
                switch (Status)
                {
                    case EAcceptedStatus::Pending:
                        URL = BaseURL + "update-status/metrics/" + DataId + "?status=Pending&build_id=" + BuildId;
                        break;
                    case EAcceptedStatus::Saved:
                        URL = BaseURL + "update-status/metrics/" + DataId + "?status=Save&build_id=" + BuildId;
                        break;
                    case EAcceptedStatus::Ignored:
                        URL = BaseURL + "update-status/metrics/" + DataId + "?status=Ignore&build_id=" + BuildId;
                        break;
                }
                break;
            case URLDefinition::Logs:
                switch (Status)
                {
                    case EAcceptedStatus::Pending:
                        URL = BaseURL + "update-status/logs/" + DataId + "?status=Pending&build_id=" + BuildId;
                        break;
                    case EAcceptedStatus::Saved:
                        URL = BaseURL + "update-status/logs/" + DataId + "?status=Save&build_id=" + BuildId;
                        break;
                    case EAcceptedStatus::Ignored:
                        URL = BaseURL + "update-status/logs/" + DataId + "?status=Ignore&build_id=" + BuildId;
                        break;
                }
                break;
        }
        break;
    case URLAction::Definition:
        switch (Definition)
        {
            case URLDefinition::Data:
                URL = BaseURL + "get-data-payload-definition/" + DataId + "?build_id=" + BuildId;
                break;
            case URLDefinition::Events:
                URL = BaseURL + "get-events-payload-definition/" + DataId + "?build_id=" + BuildId;
                break;
            case URLDefinition::Metrics:
                URL = BaseURL + "get-metrics-payload-definition/" + DataId + "?build_id=" + BuildId;
                break;
            case URLDefinition::Logs:
                URL = BaseURL + "get-logs-payload-definition/" + DataId + "?build_id=" + BuildId;
                break;
        }
        break;
    case URLAction::Receiving:
        switch (Definition)
        {
            case URLDefinition::Data:
                URL = BaseURL + "get-data-received/" + DataId + "?build_id=" + BuildId;
                break;
            case URLDefinition::Events:
                URL = BaseURL + "get-events-received/" + DataId + "?build_id=" + BuildId;
                break;
            case URLDefinition::Metrics:
                URL = BaseURL + "get-metrics-received/" + DataId + "?build_id=" + BuildId;
                break;
            case URLDefinition::Logs:
                URL = BaseURL + "get-logs-received/" + DataId + "?build_id=" + BuildId;
                break;
        }
        break;
    }

    return URL;
}

UJsonRequestObject* UPOGRSubsystem::GetJsonRequestObject()
{
    JsonObject = NewObject<UJsonRequestObject>();
    JsonObject->ConstructJsonObject();
    return JsonObject;
}

void UPOGRSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    POGRSettings = GetMutableDefault<UPOGREndpointSettings>();
}

void UPOGRSubsystem::Deinitialize()
{
    if (IsSessionActive())
    {
        DestroySession(GetSessionId());
    }
}

void UPOGRSubsystem::SendHttpRequest(const FString& URL, const UJsonRequestObject* jsonObject, const FString& SessionId)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(jsonObject->GetJsonRequestObject().ToSharedRef(), Writer);

    Request->SetURL(URL);
    Request->SetVerb(TEXT("POST"));

    Request->SetHeader("Content-Type", "application/json");
    Request->SetHeader("INTAKE_SESSION_ID", SessionId);
    Request->SetContentAsString(RequestBody);
    Request->OnProcessRequestComplete().BindUObject(this, &UPOGRSubsystem::OnResponseReceived);

    Request->ProcessRequest();
}

void UPOGRSubsystem::SetSessionId(FString SessionId)
{
    ActiveSessionId = SessionId;
}

void UPOGRSubsystem::SetAccessTokken(FString Payload)
{
    if (!Payload.IsEmpty())
    {
        AccessTokken = *Payload;
    }
}

void UPOGRSubsystem::SetIsUserLoggedIn(bool LoginStatus)
{
    IsLoggedIn = LoginStatus;
}

void UPOGRSubsystem::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
    if (!bConnectedSuccessfully) {
        UE_LOG(LogTemp, Error, TEXT("(Null Response) bConnectedSuccessfully: %i"), bConnectedSuccessfully);
        return;
    }

    TSharedPtr<FJsonObject> ResponseObj;
    if (Response != NULL) {
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
        FJsonSerializer::Deserialize(Reader, ResponseObj);
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("(Null Response)"));
    }

    // Logging Data in Output Log Window Received from Backend.
    if (Response != NULL) {
        UE_LOG(LogTemp, Display, TEXT("Data-ContentAsString: %s"), *Response->GetContentAsString());
        UE_LOG(LogTemp, Display, TEXT("Data-URL: %s"), *Response->GetURL());
    }
}

void UPOGRSubsystem::Login()
{
    FWebSocketsModule& WebSocketModule = FModuleManager::LoadModuleChecked<FWebSocketsModule>("WebSockets");
    WebSocket = WebSocketModule.CreateWebSocket(FString("wss://developer-websocket-stage.pogr.io/ws"));

    if (WebSocket.IsValid())
    {
        /* *Binding Delagetes for the Callbacks */
        WebSocket->OnConnected().AddUObject(this, &UPOGRSubsystem::OnWebSocketConnected);
        WebSocket->OnConnectionError().AddUObject(this, &UPOGRSubsystem::OnWebSocketError);
        WebSocket->OnClosed().AddUObject(this, &UPOGRSubsystem::OnWebSocketClosed);
        WebSocket->OnMessage().AddUObject(this, &UPOGRSubsystem::OnWebSocketMessage);

        WebSocket->Connect(); // Connecting with the Server
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create WebSocket object for URL: %s"));
    }
}

void UPOGRSubsystem::OnWebSocketConnected()
{
    UE_LOG(LogTemp, Log, TEXT("WebSocket connected successfully!"));
}

void UPOGRSubsystem::OnWebSocketError(const FString& Error)
{
    UE_LOG(LogTemp, Error, TEXT("WebSocket error: %s"), *Error);
}

void UPOGRSubsystem::OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
    UE_LOG(LogTemp, Log, TEXT("WebSocket closed with status code: %d, reason: %s"), StatusCode, *Reason);
}

void UPOGRSubsystem::OnWebSocketMessage(const FString& Message)
{
    // Parse the JSON string into a JSON object
    TSharedPtr<FJsonObject> JsonDataObject;
    TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Message);

    if (FJsonSerializer::Deserialize(JsonReader, JsonDataObject))
    {
        // Check if the JSON object contains the "payload" field
        if (JsonDataObject->HasField("payload"))
        {
            // Get the "payload" object
            TSharedPtr<FJsonObject> PayloadObject = JsonDataObject->GetObjectField("payload");

            // Check if the "payload" object contains the "redirect_url" field
            if (PayloadObject->HasField("redirect_url"))
            {
                // Get the value of the "redirect_url" field
                FString RedirectUrl = PayloadObject->GetStringField("redirect_url");

                if (!RedirectUrl.IsEmpty())
                    FPlatformProcess::LaunchURL(*RedirectUrl, nullptr, nullptr);
            }
            
            // Check if the "payload" object contains the "access_token" field
            if (PayloadObject->HasField("access_token"))
            {
                // Get the value of the "access_token" field
                AccessTokken = PayloadObject->GetStringField("access_token");

                if (!AccessTokken.IsEmpty())
                {
                    SetAccessTokken(AccessTokken);
                    SetIsUserLoggedIn(true);
                    OnLoginComplete.Broadcast(GetIsUserLoggedIn());
                }

                WebSocket->Close();
            }
        }
        else
            UE_LOG(LogTemp, Warning, TEXT("JSON object does not contain the 'payload' field"));
    }
    else
        UE_LOG(LogTemp, Error, TEXT("Failed to deserialize JSON string"));
}

void UPOGRSubsystem::SetContentAsString(FString Content)
{
    ContentAsString = Content;
    OnPayloadCallback.Broadcast(ContentAsString);
}

void UPOGRSubsystem::SetPayloadDatatype(URLDefinition PayloadDatatype)
{
    PayloadDefinition = PayloadDatatype;
    OnPayloadDataUpdate.Broadcast();
}

const FString UPOGRSubsystem::GetSelectedOptionValue() const
{
    switch (GetPayloadDatatype()) {
        case URLDefinition::Data: return FString("Data"); break;
        case URLDefinition::Events: return FString("Event"); break;
        case URLDefinition::Metrics: return FString("Metric"); break;
        case URLDefinition::Logs: return FString("Log"); break;
    }
    return FString("None Selected");
}

void UPOGRSubsystem::SetOrganizationOption(FString OrganizationValue)
{
    OrganizationName = OrganizationValue;

    for (const auto Organization : GetOrganizationDataArray())
    {
        if (OrganizationName == Organization.Name)
        {
            GetOrganizationGameData(POGR_CLIENT, POGR_SESSION, Organization.UUID);
        }
    }
}

FString UPOGRSubsystem::ConstructOrgGameURL(const FString& Id)
{
    FString BaseURL = FString("https://api-stage.pogr.io/v1//organizations/");
    // Construct the complete URL
    FString URL = BaseURL + Id + "/plugins/games";
    return URL;
}

void UPOGRSubsystem::SetGameOption(FString GameValue)
{
    GameTitle = GameValue;
}

/*
    - Break the URL to Understand how many parameters are required to fullfill the Command 
    * Listing URL required two things - what need to be list down - Build Id
    * Update URL required four things - what need to be updated - Id of the Data Defination - status - build Id
    * Data Defination URL requried three things - what need to be defined - Id of the Data Defination - build Id
    * Data Received URL required three things - what need to be received - Id of the Data Defination - build Id
*/

/*
    * {{POGR_API}}{{API_VERSION}}widget/data/developer-review/
    list-data-payloads?build_id=8216bfc4738d3789f684f9d3f117935e5af49f04c8baa6ed70d11a8e686d764c315e439dc6ee4d6abf52d333a3ca64a246e554bb0709255dbfc287deb311ff8b
    * {{POGR_API}}{{API_VERSION}}widget/data/developer-review/
    update-status/data/65c875999de8d314e9f5c1d3?status=Save&build_id=8216bfc4738d3789f684f9d3f117935e5af49f04c8baa6ed70d11a8e686d764c315e439dc6ee4d6abf52d333a3ca64a246e554bb0709255dbfc287deb311ff8b
    * {{POGR_API}}{{API_VERSION}}widget/data/developer-review/
    get-data-payload-definition/65efe09d5753fce8bad793bf?build_id=8216bfc4738d3789f684f9d3f117935e5af49f04c8baa6ed70d11a8e686d764c315e439dc6ee4d6abf52d333a3ca64a246e554bb0709255dbfc287deb311ff8b
    * {{POGR_API}}{{API_VERSION}}widget/data/developer-review/
    get-data-received/65c8773d9de8d314e9f5c1d4?build_id=8216bfc4738d3789f684f9d3f117935e5af49f04c8baa6ed70d11a8e686d764c315e439dc6ee4d6abf52d333a3ca64a246e554bb0709255dbfc287deb311ff8b

*/

// {{POGR_API}}{{API_VERSION}}widget/data/developer-review/get-logs-received/?build_id=8216bfc4738d3789f684f9d3f117935e5af49f04c8baa6ed70d11a8e686d764c315e439dc6ee4d6abf52d333a3ca64a246e554bb0709255dbfc287deb311ff8b