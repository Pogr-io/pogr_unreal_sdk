#include "POGRSettings.h"
#include "Internationalization/Regex.h"

UPOGREndpointSettings::UPOGREndpointSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

FString UPOGREndpointSettings::GetInitEndpoint() const
{
	FString HostString = FString();
	if (!InitEndpoint.IsEmpty())
		HostString = InitEndpoint;
	if (IsValidIPAddress(HostString) || IsValidURL(HostString))
		return HostString;
	UE_LOG(LogTemp, Error, TEXT("POGR InitEndpoint is not defined or incorrect IP/URL under Plugin (POGR) Project Settings"));
	return HostString = "Init Endpoint is not Defined.";
}

FString UPOGREndpointSettings::GetShutdownEndpoint() const
{
	FString HostString = FString();
	if (!ShutdownEndpoint.IsEmpty())
		HostString = ShutdownEndpoint;
	if (IsValidIPAddress(HostString) || IsValidURL(HostString))
		return HostString;
	UE_LOG(LogTemp, Error, TEXT("POGR Shutdown Endpoint is not defined or incorrect IP/URL under Plugin (POGR) Project Settings"));
	return HostString = "Shutdown Endpoint is not Defined.";
}

FString UPOGREndpointSettings::GetEventEndpoint() const
{
	FString HostString = FString();
	if (!EventEndpoint.IsEmpty())
		HostString = EventEndpoint;
	if (IsValidIPAddress(HostString) || IsValidURL(HostString))
		return HostString;
	UE_LOG(LogTemp, Error, TEXT("POGR Event Endpoint is not defined or incorrect IP/URL under Plugin (POGR) Project Settings"));
	return HostString = "Event Endpoint is not Defined.";
}

FString UPOGREndpointSettings::GetDataEndpoint() const
{
	FString HostString = FString();
	if (!DataEndpoint.IsEmpty())
		HostString = DataEndpoint;
	if (IsValidIPAddress(HostString) || IsValidURL(HostString))
		return HostString;
	UE_LOG(LogTemp, Error, TEXT("POGR Data Endpoint is not defined or incorrect IP/URL under Plugin (POGR) Project Settings"));
	return HostString = "Data Endpoint is not Defined.";
}

FString UPOGREndpointSettings::GetLogsEndpoint() const
{
	FString HostString = FString();
	if (!LogsEndpoint.IsEmpty())
		HostString = LogsEndpoint;
	if (IsValidIPAddress(HostString) || IsValidURL(HostString))
		return HostString;
	UE_LOG(LogTemp, Error, TEXT("POGR Logs Endpoint is not defined or incorrect IP/URL under Plugin (POGR) Project Settings"));
	return HostString = "Logs Endpoint is not Defined.";
}

FString UPOGREndpointSettings::GetMetricsEndpoint() const
{
	FString HostString = FString();
	if (!MetricsEndpoint.IsEmpty())
		HostString = MetricsEndpoint;
	if (IsValidIPAddress(HostString) || IsValidURL(HostString))
		return HostString;
	UE_LOG(LogTemp, Error, TEXT("POGR Metrics Endpoint is not defined or incorrect IP/URL under Plugin (POGR) Project Settings"));
	return HostString = "Metrics Endpoint is not Defined.";
}

FString UPOGREndpointSettings::GetMonitorEndpoint() const
{
	FString HostString = FString();
	if (!MonitorEndpoint.IsEmpty())
		HostString = MonitorEndpoint;
	if (IsValidIPAddress(HostString) || IsValidURL(HostString))
		return HostString;
	UE_LOG(LogTemp, Error, TEXT("POGR Monitor Endpoint is not defined or incorrect IP/URL under Plugin (POGR) Project Settings"));
	return HostString = "Monitor Endpoint is not Defined.";
}

FString UPOGREndpointSettings::GetOrgDataEndpoint() const
{
	FString HostString = FString();
	if (!OrganizationDataEndpoint.IsEmpty())
		HostString = OrganizationDataEndpoint;
	if (IsValidIPAddress(HostString) || IsValidURL(HostString))
		return HostString;
	UE_LOG(LogTemp, Error, TEXT("POGR Organization Data Endpoint is not defined or incorrect IP/URL under Plugin (POGR) Project Settings"));
	return HostString = "Organization Data Endpoint is not Defined.";
}

FString UPOGREndpointSettings::GetPogrUrlDefinationEndpoint() const
{
	FString HostString = FString();
	if (!POGRUrlDefinationEndpoint.IsEmpty())
		HostString = POGRUrlDefinationEndpoint;
	if (IsValidIPAddress(HostString) || IsValidURL(HostString))
		return HostString;
	UE_LOG(LogTemp, Error, TEXT("POGR Generic URL Endpoint is not defined or incorrect IP/URL under Plugin (POGR) Project Settings"));
	return HostString = "POGR Generic URL Endpoint is not Defined.";
}

FString UPOGREndpointSettings::GetOrgGameDataEndpoint() const
{
	FString HostString = FString();
	if (!GameDataEndpoint.IsEmpty())
		HostString = GameDataEndpoint;
	if (IsValidIPAddress(HostString) || IsValidURL(HostString))
		return HostString;
	UE_LOG(LogTemp, Error, TEXT("POGR Organization Game Data Endpoint is not defined or incorrect IP/URL under Plugin (POGR) Project Settings"));
	return HostString = "Organization Game Data Endpoint is not Defined.";
}

FString UPOGREndpointSettings::GetOrgGameEndpoint() const
{
	FString HostString = FString();
	if (!OragnizationGameEndpoint.IsEmpty())
		HostString = OragnizationGameEndpoint;
	if (IsValidIPAddress(HostString) || IsValidURL(HostString))
		return HostString;
	UE_LOG(LogTemp, Error, TEXT("POGR Organization Game Endpoint is not defined or incorrect IP/URL under Plugin (POGR) Project Settings"));
	return HostString = "Organization Game Endpoint is not Defined.";
}

FString UPOGREndpointSettings::GetProfileDataEndpoint() const
{
	FString HostString = FString();
	if (!ProfileDataEndpoint.IsEmpty())
		HostString = ProfileDataEndpoint;
	if (IsValidIPAddress(HostString) || IsValidURL(HostString))
		return HostString;
	UE_LOG(LogTemp, Error, TEXT("POGR Profile Data Endpoint is not defined or incorrect IP/URL under Plugin (POGR) Project Settings"));
	return HostString = "Profile Data Endpoint is not Defined.";
}

bool UPOGREndpointSettings::IsValidIPAddress(const FString& IPAddressString) const
{
	const FRegexPattern IPv4Pattern(TEXT("^\\b(?:\\d{1,3}\\.){3}\\d{1,3}\\b"));
	FRegexMatcher IPv4Matcher(IPv4Pattern, IPAddressString);
	return IPv4Matcher.FindNext();
}

bool UPOGREndpointSettings::IsValidURL(const FString& URLString) const 
{
	const FRegexPattern URLPattern(TEXT(
		"^(https?|ftp):\\/\\/"
		"((([a-zA-Z0-9-]+\\.)+[a-zA-Z]{2,})|((\\d{1,3}\\.){3}\\d{1,3}))"
		"(:\\d{1,5})?(\\/[^\\s]*)?$"));

	FRegexMatcher URLMatcher(URLPattern, URLString);
	return URLMatcher.FindNext();
}
