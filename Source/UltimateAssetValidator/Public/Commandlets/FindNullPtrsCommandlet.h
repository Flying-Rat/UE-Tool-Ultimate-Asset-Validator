// Copyright 2022 Tadeáš Anděl, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "FindNullPtrsCommandlet.generated.h"

UCLASS(BlueprintType, Blueprintable)
class ULTIMATEASSETVALIDATOR_API UFindNullPtrsCommandlet : public UCommandlet
{
	GENERATED_BODY()

public: // Construction

	UFindNullPtrsCommandlet();

public: // Commandlet
	
	virtual int32 Main(const FString& FullCommandLine) override;

private: // Validation

	static void FindNullPtrProperties(void* FromContainer, const UStruct* InStruct, TArray<FString>& OutNullPtrProperties);
	static bool DoesPropertyContainNullPtrNoContainer(FProperty* Property, void* FromContainer);
	static bool DoesArrayPropertyContainNullPtr(FProperty* ArrayInnerProperty, void* ArrayElementPtr);
	static bool ShouldSearchArrayProperty(FArrayProperty* ArrayProperty);
	static bool ShouldSearchMapProperty(FMapProperty* MapProperty);
	static bool ShouldSearchSetProperty(FSetProperty* SetProperty);
	static bool IsPtrProperty(FProperty* Property);

private: // Constants
	
	inline static const TCHAR* NullPtrMetaDataFlag{TEXT("NotNullptr")};
	inline static const TCHAR* SkipUAVMetaDataFlag{TEXT("SkipUAV")};
};