// Copyright Epic Games, Inc. All Rights Reserved.

#include "UltimateAssetValidator.h"

#define LOCTEXT_NAMESPACE "FUltimateAssetValidatorModule"

void FUltimateAssetValidatorModule::StartupModule() { }

void FUltimateAssetValidatorModule::ShutdownModule() { }

DEFINE_LOG_CATEGORY(LogUAV)

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUltimateAssetValidatorModule, UltimateAssetValidator)