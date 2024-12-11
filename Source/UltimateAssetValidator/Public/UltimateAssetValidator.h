// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

ULTIMATEASSETVALIDATOR_API DECLARE_LOG_CATEGORY_EXTERN(LogUAV, Log, All);

class FUltimateAssetValidatorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
