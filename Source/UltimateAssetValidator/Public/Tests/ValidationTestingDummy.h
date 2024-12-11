// Copyright 2022 Tadeáš Anděl, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ValidationTestingDummy.generated.h"

USTRUCT(Blueprintable, BlueprintType)
struct FNestedValidationTestingDummyStruct
{
	GENERATED_BODY()

public: // Validation Tests
	
	UPROPERTY(EditAnywhere, meta = (NotNullptr), Category = "Tests")
	UObject* BadProperty1;

	UPROPERTY(EditAnywhere, meta = (NotNullptr), Category = "Tests")
	TArray<UObject*> BadProperty2;

	UPROPERTY(EditAnywhere, meta = (NotNullptr), Category = "Tests")
	UObject* GoodProperty1;
};

USTRUCT(Blueprintable, BlueprintType)
struct FValidationTestingDummyStruct
{
	GENERATED_BODY()

public: // Validation Tests
	
	UPROPERTY(EditAnywhere, meta = (NotNullptr), Category = "Tests")
	UObject* BadProperty1;
	
	UPROPERTY(EditAnywhere, Category = "Tests")
	FNestedValidationTestingDummyStruct NestedStruct;

	UPROPERTY(EditAnywhere, meta = (NotNullptr), Category = "Tests")
	UObject* GoodProperty1;
};

UCLASS(BlueprintType, Blueprintable)
class ULTIMATEASSETVALIDATOR_API AValidationTestingDummy : public AActor
{
	GENERATED_BODY()

public: // Construction

	AValidationTestingDummy();

public: // Validation Tests

	UPROPERTY(EditAnywhere, meta = (NotNullptr), Category = "Tests")
	UObject* BadProperty1;

	UPROPERTY(EditAnywhere, meta = (NotNullptr), Category = "Tests")
	TSubclassOf<UObject> BadProperty2;

	UPROPERTY(EditAnywhere, meta = (NotNullptr), Category = "Tests")
	TArray<UObject*> BadProperty3;

	UPROPERTY(EditAnywhere, Category = "Tests")
	FValidationTestingDummyStruct BadProperty4;

	UPROPERTY(EditAnywhere, Category = "Tests")
	TArray<FValidationTestingDummyStruct> BadProperty5;

	UPROPERTY(EditAnywhere, Category = "Tests")
	TMap<FString, FValidationTestingDummyStruct> BadMapProperty6;
	
	UPROPERTY(EditAnywhere, meta = (NotNullptr), Category = "Tests")
	TSet<UObject*> BadSetProperty7;
	
	UPROPERTY(EditAnywhere, meta = (NotNullptr), Category = "Tests")
	UObject* GoodProperty1;

	UPROPERTY(EditAnywhere, meta = (NotNullptr), Category = "Tests")
	TSubclassOf<UObject> GoodProperty2;

	UPROPERTY(EditAnywhere, meta = (NotNullptr), Category = "Tests")
	TArray<UObject*> GoodProperty3;

	UPROPERTY(EditAnywhere, Category = "Tests")
	FValidationTestingDummyStruct GoodProperty4;

	UPROPERTY(EditAnywhere, meta = (NotNullptr), Category = "Tests")
	TArray<int> IrrelevantProperty;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tests")
	class UValidationTestingDummyComponent* DummyTester;
};

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class ULTIMATEASSETVALIDATOR_API UValidationTestingDummyComponent : public UActorComponent
{
	GENERATED_BODY()

public: // Validation Tests

	UPROPERTY(EditAnywhere, meta = (NotNullptr), Category = "Tests")
	UObject* BadProperty1;

	UPROPERTY(EditAnywhere, meta = (NotNullptr), Category = "Tests")
	TSubclassOf<UObject> BadProperty2;

	UPROPERTY(EditAnywhere, meta = (NotNullptr), Category = "Tests")
	TArray<UObject*> BadProperty3;

	UPROPERTY(EditAnywhere, meta = (NotNullptr), Category = "Tests")
	UObject* GoodProperty1;

	UPROPERTY(EditAnywhere, meta = (NotNullptr), Category = "Tests")
	TSubclassOf<UObject> GoodProperty2;

	UPROPERTY(EditAnywhere, meta = (NotNullptr), Category = "Tests")
	TArray<UObject*> GoodProperty3;
};

UCLASS(BlueprintType, Blueprintable)
class ULTIMATEASSETVALIDATOR_API UValidationTestingDummyDataAsset : public UDataAsset
{
	GENERATED_BODY()

public: // Validation Tests

	UPROPERTY(EditAnywhere, meta = (NotNullptr), Category = "Tests")
	UObject* BadProperty1;

	UPROPERTY(EditAnywhere, meta = (NotNullptr), Category = "Tests")
	TSubclassOf<UObject> BadProperty2;

	UPROPERTY(EditAnywhere, meta = (NotNullptr), Category = "Tests")
	TArray<UObject*> BadProperty3;

	UPROPERTY(EditAnywhere, meta = (NotNullptr), Category = "Tests")
	UObject* GoodProperty1;

	UPROPERTY(EditAnywhere, meta = (NotNullptr), Category = "Tests")
	TSubclassOf<UObject> GoodProperty2;

	UPROPERTY(EditAnywhere, meta = (NotNullptr), Category = "Tests")
	TArray<UObject*> GoodProperty3;
};

UENUM(BlueprintType)
enum class EValidationEnumTest : uint8
{
	First,
	Second,
	Third,
};

USTRUCT(Blueprintable, BlueprintType)
struct FStructInMapTest
{
	GENERATED_BODY()

public: // Data

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (NotNullptr), Category = "Tests")
	TSubclassOf<UObject> BadProperty;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (NotNullptr), Category = "Tests")
	TSubclassOf<UObject> GoodProperty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (NotNullptr), Category = "Tests")
	TMap<FString, TSubclassOf<UObject>> StructMap;
};

UCLASS(BlueprintType, Blueprintable)
class ULTIMATEASSETVALIDATOR_API UValidationTestingDummyDataAsset2 : public UDataAsset
{
	GENERATED_BODY()

public: // Data

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tests")
	TMap<FString, FStructInMapTest> EasyMap;
};
