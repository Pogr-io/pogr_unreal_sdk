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