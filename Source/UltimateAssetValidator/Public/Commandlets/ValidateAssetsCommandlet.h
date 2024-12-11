// Copyright 2022 Tadeáš Anděl, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "ValidateAssetsCommandlet.generated.h"

UCLASS(BlueprintType, Blueprintable)
class ULTIMATEASSETVALIDATOR_API UValidateAssetsCommandlet : public UCommandlet
{
	GENERATED_BODY()

public: // Construction

	UValidateAssetsCommandlet();

public: // Commandlet
	
	virtual int32 Main(const FString& FullCommandLine) override;
};
