// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "POGRSettings.h"

class FPOGRModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/**
	* Singleton-like access to this module's interface. This is just for
	* convenience! Beware of calling this during the shutdown phase, though.
	* Your module might have been unloaded already.
	*
	* @return Returns singleton instance, loading the module on demand if needed
	*/
	static inline FPOGRModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FPOGRModule>("POGR");
	}
	/**
	* Singleton-like access to this module's interface. This is just for
	* convenience! Beware of calling this during t c he shutdown phase, though.
	* Your module might have been unloaded already.
	*
	* @return Returns singleton instance, loading the module on demand if needed
	*/
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("POGR");
	}

	/**POGR Developer Settings for the Endpoint*/
	void RegisterPOGREndpointSettings();
	void UnregisterPOGREndpointSettings();

	/** Getter for internal settings object to support runtime configuration changes */
	UPOGREndpointSettings* GetSettings() const;

private:
	UPOGREndpointSettings* ModuleSettings;
};
