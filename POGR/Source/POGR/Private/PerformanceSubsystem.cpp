#include "PerformanceSubsystem.h"
#include "POGR.h"
#include "WebSockets/Public/IWebSocket.h"
#include "WebSockets/Public/WebSocketsModule.h"
#include "WebSockets/Public/IWebSocketsManager.h"
#include "JsonUtilities/Public/JsonObjectConverter.h"


/**Discarding the Commented Methods as the implementation in them is obsolete for Production*/

//const FPerformanceData UPerformanceSubsystem::GetPOGRPerformanceDump() const
//{
//    FPerformanceData POGRPerformanceDump;
//
//    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
//
//    POGRPerformanceDump.UsedPhysical = (float)MemoryStats.UsedPhysical / 1024;
//    POGRPerformanceDump.AvailablePhysical = (float)MemoryStats.AvailablePhysical / 1024;
//    POGRPerformanceDump.AvailableVirtual = (float)MemoryStats.AvailableVirtual / 1024;
//    POGRPerformanceDump.UsedVirtual = (float)MemoryStats.UsedVirtual / 1024;
//    POGRPerformanceDump.PeakUsedPhysical = (float)MemoryStats.PeakUsedPhysical / 1024;
//    POGRPerformanceDump.PeakUsedVirtual = (float)MemoryStats.PeakUsedVirtual / 1024;
//
//    POGRPerformanceDump.CPUBrand = FWindowsPlatformMisc::GetCPUVendor();
//    POGRPerformanceDump.CPUName = FWindowsPlatformMisc::GetCPUBrand();
//    POGRPerformanceDump.GPUBrand = FWindowsPlatformMisc::GetPrimaryGPUBrand();
//    POGRPerformanceDump.OSVersion = FWindowsPlatformMisc::GetOSVersion();
//
//    return POGRPerformanceDump;
//}

void UPerformanceSubsystem::SendMetricsEvent(const UJsonRequestObject* jsonObject, const FString& SessionID)
{

    //// Getting POGR Module Settings
    //if (FPOGRModule::IsAvailable())
    //    POGRSettings = FPOGRModule::Get().GetSettings();

    //// Creating Http Request 
    //TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    //TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();

    ///** Update Values in Request Object*/
    //RequestObj->SetStringField("service", "game_server");
    //RequestObj->SetStringField("environment", "production");
    //RequestObj->SetStringField("CPUBrand", PerformanceData.CPUBrand);
    //RequestObj->SetStringField("CPUName", PerformanceData.CPUName);
    //RequestObj->SetStringField("GPUBrand", PerformanceData.GPUBrand);
    //RequestObj->SetStringField("OSVersion", PerformanceData.OSVersion);
    //RequestObj->SetNumberField("UsedPhysical", PerformanceData.UsedPhysical);
    //RequestObj->SetNumberField("AvailablePhysical", PerformanceData.AvailablePhysical);
    //RequestObj->SetNumberField("AvailableVirtual", PerformanceData.AvailableVirtual);
    //RequestObj->SetNumberField("UsedVirtual", PerformanceData.UsedVirtual);
    //RequestObj->SetNumberField("PeakUsedVirtual", PerformanceData.PeakUsedVirtual);
    //RequestObj->SetNumberField("PeakUsedPhysical", PerformanceData.PeakUsedPhysical);
    ///** End of Request Object*/

    //// Writing the Data to Object for Request
    //FString RequestBody;
    //TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    //FJsonSerializer::Serialize(RequestObj, Writer);

    //Request->SetURL(POGRSettings->GetMetricsEndpoint());
    //// Sending Request Via POST Method
    //Request->SetVerb(TEXT("POST")); // Is this a GET | POST | PUT | PATCH | DELETE Request.

    //Request->SetHeader("Content-Type", "application/json");
    //Request->SetHeader("INTAKE_SESSION_ID", SessionID);
    //Request->SetContentAsString(RequestBody);
    //Request->OnProcessRequestComplete().BindUObject(this, &UPerformanceSubsystem::OnResponseReceived);

    //Request->ProcessRequest(); // Once Request Is Ready To Send Just Call This Method.
}

void UPerformanceSubsystem::SendInitSessionEvent(const FString& ClientId, const FString& BuildId)
{
}

void UPerformanceSubsystem::SendEndSessionEvent(const FString& SessionID, const UJsonRequestObject* JsonObjectValue)
{
}

void UPerformanceSubsystem::SendEvent(const FString& SessionID)
{
}

void UPerformanceSubsystem::SendDataEvent(const FString& SessionID)
{
}

void UPerformanceSubsystem::SendLogsEvent(const FString& SessionID)
{
}

void UPerformanceSubsystem::SendMonitorEvent(const FString& SessionID)
{
}

UJsonRequestObject* UPerformanceSubsystem::GetJsonRequestObject()
{
    JsonObject = NewObject<UJsonRequestObject>();
    JsonObject->ConstructJsonObject();
    return JsonObject;
}

void UPerformanceSubsystem::SetSessionId(FString SessionID)
{
    SessionId = SessionID;
}

void UPerformanceSubsystem::SetAccessTokken(FString RawData)
{
}

void UPerformanceSubsystem::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
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

    // Logging Data in Output Log File Received from Backend.
    if (Response != NULL) {
        UE_LOG(LogTemp, Display, TEXT("Data-ContentAsString: %s"), *Response->GetContentAsString());
        UE_LOG(LogTemp, Display, TEXT("Data-URL: %s"), *Response->GetURL());
    }
}


// **************************************************************************************

void UPerformanceSubsystem::SendTestMethod(const UJsonRequestObject* jsonObject)
{
    // Creating Http Request 
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();
    RequestObj->SetObjectField("Event Discription", jsonObject->GetJsonRequestObject());

    // Writing the Data to Object for Request
    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(RequestObj, Writer);

    Request->SetURL(FString("https://jsonplaceholder.typicode.com/posts"));
    // Sending Request Via POST Method
    Request->SetVerb(TEXT("POST")); // Is this a GET | POST | PUT | PATCH | DELETE Request.

    Request->SetHeader("Content-Type", "application/json");
    /*Request->SetHeader("INTAKE_SESSION_ID", SessionID);*/
    Request->SetContentAsString(RequestBody);
    Request->OnProcessRequestComplete().BindUObject(this, &UPerformanceSubsystem::OnResponseReceived);

    Request->ProcessRequest(); // Once Request Is Ready To Send Just Call This Method.
}

void UPerformanceSubsystem::SendTestEvent(const TArray<UJsonRequestObject*> jsonObject)
{
    // Creating Http Request 
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();
    for (const auto jsonObjectElement : jsonObject)
    {
        if (jsonObjectElement != nullptr)
        {
            if (!jsonObjectElement->GetJsonRequestObjectName().IsEmpty())
            {
                RequestObj->SetObjectField(jsonObjectElement->GetJsonRequestObjectName(), jsonObjectElement->GetJsonRequestObject());
            }
            else
            {
                // Crash Logic from the Bee project or from the plugin third party template.
            }
        }
    }

    // Writing the Data to Object for Request
    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(RequestObj, Writer);

    Request->SetURL(FString("https://jsonplaceholder.typicode.com/posts"));
    // Sending Request Via POST Method
    Request->SetVerb(TEXT("POST")); // Is this a GET | POST | PUT | PATCH | DELETE Request.

    Request->SetHeader("Content-Type", "application/json");
    /*Request->SetHeader("INTAKE_SESSION_ID", SessionID);*/
    Request->SetContentAsString(RequestBody);
    Request->OnProcessRequestComplete().BindUObject(this, &UPerformanceSubsystem::OnResponseReceived);

    Request->ProcessRequest(); // Once Request Is Ready To Send Just Call This Method.
}

void UPerformanceSubsystem::SendDataEventAsync(const TArray<UJsonRequestObject*> jsonObject)
{

}

const FString UPerformanceSubsystem::GetCPUName() const
{
    return FWindowsPlatformMisc::GetCPUBrand();
}

const FString UPerformanceSubsystem::GetCPUBrand() const
{
    return FWindowsPlatformMisc::GetCPUVendor();
}

const FString UPerformanceSubsystem::GetGPUName() const
{
    return FWindowsPlatformMisc::GetPrimaryGPUBrand();
}

const FString UPerformanceSubsystem::GetOSVersion() const
{
    return FWindowsPlatformMisc::GetOSVersion();
}

const FString UPerformanceSubsystem::GetUsedPhysicalMemory() const
{
    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
    float UsedPhysicalMemory = (float)MemoryStats.UsedPhysical / (1024 * 1024 * 1024);
    FString UsedPhysicalString = FString::Printf(TEXT("%.2f GB"), UsedPhysicalMemory);
    return UsedPhysicalString;
}

const FString UPerformanceSubsystem::GetAvailablePhysicalMemory() const
{
    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
    float AvailablePhysicalMemory = (float)MemoryStats.AvailablePhysical / (1024 * 1024 * 1024);
    FString UsedAvailablePhysicalMemory = FString::Printf(TEXT("%.2f GB"), AvailablePhysicalMemory);
    return UsedAvailablePhysicalMemory;
}

const FString UPerformanceSubsystem::GetAvailableVirtualMemory() const
{
    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
    float AvailableVirtualMemory = (float)MemoryStats.AvailableVirtual / (1024 * 1024 * 1024);
    FString UsedAvailableVirtualMemory = FString::Printf(TEXT("%.2f GB"), AvailableVirtualMemory);
    return UsedAvailableVirtualMemory;
}

const FString UPerformanceSubsystem::GetUsedVirtualMemory() const
{
    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
    float UsedVirtualMemory = (float)MemoryStats.UsedVirtual / (1024 * 1024 * 1024);
    FString UsedVirtualMemoryString = FString::Printf(TEXT("%.2f GB"), UsedVirtualMemory);
    return UsedVirtualMemoryString;
}

const FString UPerformanceSubsystem::GetPeakUsedVirtualMemory() const
{
    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
    float PeakUsedVirtualMemory = (float)MemoryStats.PeakUsedVirtual / (1024 * 1024 * 1024);
    FString PeakUsedVirtualMemoryString = FString::Printf(TEXT("%.2f GB"), PeakUsedVirtualMemory);
    return PeakUsedVirtualMemoryString;
}

const FString UPerformanceSubsystem::GetPeakUsedPhysicalMemory() const
{
    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
    float PeakUsedPhysicalMemory = (float)MemoryStats.PeakUsedPhysical / (1024 * 1024 * 1024);
    FString PeakUsedPhysicalMemoryString = FString::Printf(TEXT("%.2f GB"), PeakUsedPhysicalMemory);
    return PeakUsedPhysicalMemoryString;
}

void UPerformanceSubsystem::Login()
{
    FWebSocketsModule& WebSocketModule = FModuleManager::LoadModuleChecked<FWebSocketsModule>("WebSockets");
    TSharedPtr<IWebSocket> WebSocket = WebSocketModule.CreateWebSocket(FString("wss://developer-websocket-stage.pogr.io/ws"));

    if (WebSocket.IsValid())
    {
        /* *Binding Delagetes for the Callbacks */
        WebSocket->OnConnected().AddUObject(this, &UPerformanceSubsystem::OnWebSocketConnected);
        WebSocket->OnConnectionError().AddUObject(this, &UPerformanceSubsystem::OnWebSocketError);
        WebSocket->OnClosed().AddUObject(this, &UPerformanceSubsystem::OnWebSocketClosed);
        WebSocket->OnMessage().AddUObject(this, &UPerformanceSubsystem::OnWebSocketMessage);

        WebSocket->Connect(); // Connecting with the Server
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create WebSocket object for URL: %s"));
    }
}

void UPerformanceSubsystem::OnWebSocketConnected()
{
    UE_LOG(LogTemp, Log, TEXT("WebSocket connected successfully!"));
}

void UPerformanceSubsystem::OnWebSocketError(const FString& Error)
{
    UE_LOG(LogTemp, Error, TEXT("WebSocket error: %s"), *Error);
}

void UPerformanceSubsystem::OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
    UE_LOG(LogTemp, Log, TEXT("WebSocket closed with status code: %d, reason: %s"), StatusCode, *Reason);
}

void UPerformanceSubsystem::OnWebSocketMessage(const FString& Message)
{
    UE_LOG(LogTemp, Log, TEXT("Received WebSocket message: %s"), *Message);

    // Assuming the received message contains a URL
    // You can implement your parsing logic here
    // 
    //FString ParsedURL = ParseURLFromMessage(Message);
    //if (!ParsedURL.IsEmpty())
    //{
    //    // Open the URL using Unreal's built-in functionality
    //    FPlatformProcess::LaunchURL(*ParsedURL, nullptr, nullptr);
    //}
}