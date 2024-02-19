#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "POGRSettings.generated.h"

UCLASS(config = Engine, defaultconfig)
class UPOGREndpointSettings : public UObject
{
	GENERATED_BODY()

public:
	UPOGREndpointSettings(const FObjectInitializer& ObjectInitializer);

	/**Get Listening Endpoint*/
	FString GetInitEndpoint() const;
	FString GetShutdownEndpoint() const;
	FString GetEventEndpoint() const;
	FString GetDataEndpoint() const;
	FString GetLogsEndpoint() const;
	FString GetMetricsEndpoint() const;
	FString GetMonitorEndpoint() const;

	/**Validate weather Endpoint is Valid*/
	bool IsValidIPAddress(const FString& IPAddressString) const;
	bool IsValidURL(const FString& URLString) const;

private:
	UPROPERTY(config, EditAnywhere, Category = Settings)
	FString InitEndpoint = "";
	
	UPROPERTY(config, EditAnywhere, Category = Settings)
	FString ShutdownEndpoint = "";

	UPROPERTY(config, EditAnywhere, Category = Settings)
	FString EventEndpoint = "";

	UPROPERTY(config, EditAnywhere, Category = Settings)
	FString DataEndpoint = "";

	UPROPERTY(config, EditAnywhere, Category = Settings)
	FString LogsEndpoint = "";

	UPROPERTY(config, EditAnywhere, Category = Settings)
	FString MetricsEndpoint = "";
	
	UPROPERTY(config, EditAnywhere, Category = Settings)
	FString MonitorEndpoint = "";

};