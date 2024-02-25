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

const FString UPOGRSubsystem::GenerateUniqueUserAssociationId() const
{
    FGuid RandomGuid = FGuid::NewGuid();
    FString AssociationId = FString::Printf(TEXT("%s"), *RandomGuid.ToString());
    return AssociationId;
}

void UPOGRSubsystem::CreateSession(const FString& ClientId, const FString& BuildId, const FString& AssociationId)
{
    if (FPOGRModule::IsAvailable())
        POGRSettings = FPOGRModule::Get().GetSettings();
    
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
    if (!POGRSettings)
    {
        if (FPOGRModule::IsAvailable())
            POGRSettings = FPOGRModule::Get().GetSettings();
    }

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

void UPOGRSubsystem::SendGameMetricsEvent(const UJsonRequestObject* jsonObject, const FString& SessionId)
{
    if (!POGRSettings)
    {
        POGRSettings = FPOGRModule::Get().GetSettings();
    }

    const auto JsonObjectData = jsonObject->GetJsonRequestObject();
    JsonObjectData->SetStringField(FString("service"), FString("game_server"));
    JsonObjectData->SetStringField(FString("environment"), FString("production"));

    FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([=]()
    {
        SendHttpRequest(POGRSettings->GetMetricsEndpoint(), jsonObject, SessionId);
    }, TStatId(), nullptr, ENamedThreads::AnyThread);
}

void UPOGRSubsystem::SendGameUserEvent(const UJsonRequestObject* jsonObject, const FString& SessionId)
{
    if (!POGRSettings)
    {
        if (FPOGRModule::IsAvailable())
            POGRSettings = FPOGRModule::Get().GetSettings();
    }

    FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([=]()
    {
        SendHttpRequest(POGRSettings->GetEventEndpoint(), jsonObject, SessionId);
    }, TStatId(), nullptr, ENamedThreads::AnyThread);
}

void UPOGRSubsystem::SendGameDataEvent(const UJsonRequestObject* jsonObject, const FString& SessionId)
{
    if (!POGRSettings)
    {
        if (FPOGRModule::IsAvailable())
            POGRSettings = FPOGRModule::Get().GetSettings();
    }

    FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([=]()
    {
        SendHttpRequest(POGRSettings->GetDataEndpoint(), jsonObject, SessionId);
    }, TStatId(), nullptr, ENamedThreads::AnyThread);
}

void UPOGRSubsystem::SendGameLogsEvent(const UJsonRequestObject* jsonObject, const FString& SessionId)
{
    if (!POGRSettings)
    {
        if (FPOGRModule::IsAvailable())
            POGRSettings = FPOGRModule::Get().GetSettings();
    }

    FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([=]()
    {
        SendHttpRequest(POGRSettings->GetLogsEndpoint(), jsonObject, SessionId);
    }, TStatId(), nullptr, ENamedThreads::AnyThread);
}

void UPOGRSubsystem::SendGamePerformanceEvent(const UJsonRequestObject* jsonObject, const FString& SessionId)
{
    if (!POGRSettings)
    {
        if (FPOGRModule::IsAvailable())
            POGRSettings = FPOGRModule::Get().GetSettings();
    }

    FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([=]()
    {
        SendHttpRequest(POGRSettings->GetMonitorEndpoint(), jsonObject, SessionId);
    }, TStatId(), nullptr, ENamedThreads::AnyThread);
}

UJsonRequestObject* UPOGRSubsystem::GetJsonRequestObject()
{
    JsonObject = NewObject<UJsonRequestObject>();
    JsonObject->ConstructJsonObject();
    return JsonObject;
}

void UPOGRSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{

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

void UPOGRSubsystem::SetTabId(FName TabId)
{
    WidgetTabId = TabId;
}