// Copyright 2022 Tadeáš Anděl, All Rights Reserved.


#include "Commandlets/FindNullPtrsCommandlet.h"

#include "UltimateAssetValidator.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"

#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"

UFindNullPtrsCommandlet::UFindNullPtrsCommandlet()
{
	FastExit = true;
	LogToConsole = true;
}

int32 UFindNullPtrsCommandlet::Main(const FString& FullCommandLine)
{
	UE_LOG(LogInit, Log, TEXT("Validating assets"));
	
	TArray<FString> Tokens;
	TArray<FString> Switches;
	TMap<FString, FString> Params;
	ParseCommandLine(*FullCommandLine, Tokens, Switches, Params);

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(AssetRegistryConstants::ModuleName);
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	TArray<FAssetData> AllAssets;
	AssetRegistry.SearchAllAssets(true);
	if (Params.Contains("AssetsDirectories"))
	{
		FString AssetsDirectoriesString(Params["AssetsDirectories"]);
		TArray<FString> AssetsDirectories;
		AssetsDirectoriesString.ParseIntoArray(AssetsDirectories, TEXT(","));
		TArray<FName> AssetsFNameDirectories;
		for (const FString& AssetsDirectory : AssetsDirectories)
		{
			AssetsFNameDirectories.Add(FName(AssetsDirectory));
		}
		AssetRegistry.GetAssetsByPaths(AssetsFNameDirectories, AllAssets, true);
	}
	else
	{
		AssetRegistry.GetAssetsByPath("/Game/", AllAssets, true);
	}

	FString NullPtrProperties;
	for (const FAssetData& AssetData : AllAssets)
	{
		UObject* LoadedAsset = AssetData.GetAsset();
		if (!IsValid(LoadedAsset))
		{
			continue;
		}

		UObject* Container;
		UClass* PropertyClass;
		UBlueprint* Blueprint = Cast<UBlueprint>(LoadedAsset);
		if (IsValid(Blueprint))
		{
			if (!IsValid(Blueprint->GeneratedClass))
			{
				continue;
			}

			PropertyClass = Blueprint->GeneratedClass;
			Container = Blueprint->GeneratedClass->GetDefaultObject();
			AActor* Actor = Cast<AActor>(Container);
			if (IsValid(Actor))
			{
				TArray<UObject*> DefaultObjectSubobjects;
				PropertyClass->GetDefaultObjectSubobjects(DefaultObjectSubobjects);

				// C++ spawned components
				for (UObject* DefaultSubObject : DefaultObjectSubobjects)
				{
					UActorComponent* ActorComponent = Cast<UActorComponent>(DefaultSubObject);
					if (!IsValid(ActorComponent))
					{
						continue;
					}
					UClass* ComponentClass = ActorComponent->GetClass();
					TArray<FString> NullPtrPropertyNames;
					FindNullPtrProperties(ActorComponent, ComponentClass, NullPtrPropertyNames);
					for (const FString& NullPtrPropertyName : NullPtrPropertyNames)
					{
						const FString& ErrorString = FString::Printf(TEXT("Nullptr property %s on component %s of actor %s"), *NullPtrPropertyName, *ActorComponent->GetName(), *AssetData.PackageName.ToString());
						UE_LOG(LogUAV, Error, TEXT("%s"), *ErrorString);
						NullPtrProperties += ErrorString + TEXT("\n");
					}
				}

				// Blueprint Spawned Components
				UBlueprintGeneratedClass* BlueprintGeneratedClass = Cast<UBlueprintGeneratedClass>(PropertyClass);
				if (IsValid(BlueprintGeneratedClass))
				{
					for (USCS_Node* Node : BlueprintGeneratedClass->SimpleConstructionScript->GetAllNodes())
					{
						if (!Node->ComponentClass->IsChildOf(UActorComponent::StaticClass()))
						{
							continue;
						}

						UActorComponent* ActorComponent = Node->ComponentTemplate;
						UClass* ComponentClass = ActorComponent->GetClass();
						TArray<FString> NullPtrPropertyNames;
						FindNullPtrProperties(ActorComponent, ComponentClass, NullPtrPropertyNames);
						for (const FString& NullPtrPropertyName : NullPtrPropertyNames)
						{
							const FString& ErrorString = FString::Printf(TEXT("Nullptr property %s on component %s of actor %s"), *NullPtrPropertyName, *ActorComponent->GetName(), *AssetData.PackageName.ToString());
							UE_LOG(LogUAV, Error, TEXT("%s"), *ErrorString);
							NullPtrProperties += ErrorString + TEXT("\n");
						}
					}
				}
			}
		}
		else
		{
			Container = LoadedAsset;
			PropertyClass = LoadedAsset->GetClass();
		}

		TArray<FString> NullPtrPropertyNames;
		FindNullPtrProperties(Container, PropertyClass, NullPtrPropertyNames);
		for (const FString& NullPtrProperty : NullPtrPropertyNames)
		{
			const FString& ErrorString = FString::Printf(TEXT("Nullptr property %s on asset %s"), *NullPtrProperty, *AssetData.PackageName.ToString());
			UE_LOG(LogUAV, Error, TEXT("%s"), *ErrorString);
			NullPtrProperties += ErrorString + TEXT("\n");
		}
	}

	if (NullPtrProperties.Len() > 0)
	{
		FString FileName = FPaths::Combine(FPaths::ProjectSavedDir(), "Logs", "NullPtrProperties.txt");
		FFileHelper::SaveStringToFile(NullPtrProperties, *FileName);
	}

	return 0; 
}

void UFindNullPtrsCommandlet::FindNullPtrProperties(void* FromContainer, const UStruct* InStruct, TArray<FString>& OutNullPtrProperties)
{
	for (TFieldIterator<FProperty> PropIt(InStruct, EFieldIterationFlags::IncludeSuper); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;
		if (Property->HasMetaData(SkipUAVMetaDataFlag))
		{
			continue;
		}
		
		const FString& PropertyName = Property->GetName();
		FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property);
		if (ArrayProperty)
		{
			if (!ShouldSearchArrayProperty(ArrayProperty))
			{
				continue;
			}
			
			FProperty* InnerProperty = ArrayProperty->Inner;
			FScriptArrayHelper Helper(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(FromContainer));
			for (int32 Index = 0; Index < Helper.Num(); ++Index)
			{
				if (DoesArrayPropertyContainNullPtr(InnerProperty, Helper.GetRawPtr(Index)))
				{
					OutNullPtrProperties.Add(FString::Printf(TEXT("%s[%d]"), *Property->GetName(), Index));
					continue;
				}

				FStructProperty* StructInnerProperty = CastField<FStructProperty>(InnerProperty);
				if (StructInnerProperty)
				{
					uint8* StructData = Helper.GetRawPtr(Index);
					TArray<FString> InnerNullPtrProperties;

					FindNullPtrProperties(StructData, StructInnerProperty->Struct, InnerNullPtrProperties);
					for (const FString& NullPtrProperty : InnerNullPtrProperties)
					{
						OutNullPtrProperties.Add(FString::Printf(TEXT("%s[%d].%s"), *Property->GetName(), Index, *NullPtrProperty));
					}
					continue;
				}
			}
			continue;
		}

		FMapProperty* MapProperty = CastField<FMapProperty>(Property);
		if (MapProperty)
		{
			if (!ShouldSearchMapProperty(MapProperty))
			{
				continue;
			}

			FProperty* ValueProperty = MapProperty->ValueProp;
			FProperty* KeyProperty = MapProperty->KeyProp;
			const uint8* ValuePtr = MapProperty->ContainerPtrToValuePtr<uint8>(FromContainer);
			
			FScriptMapHelper MapHelper(MapProperty, ValuePtr);
			for (FScriptMapHelper::FIterator MapIt = MapHelper.CreateIterator(); MapIt; ++MapIt)
			{
				int32 IteratorIndex = *MapIt;
				uint8* KeyData = MapHelper.GetKeyPtr(IteratorIndex);
				FString KeyValue;
				KeyProperty->ExportText_Direct(KeyValue, KeyData, KeyData, nullptr, PPF_None);
				
				uint8* ValueData = MapHelper.GetValuePtr(IteratorIndex);
				if (DoesArrayPropertyContainNullPtr(ValueProperty, ValueData))
				{
					OutNullPtrProperties.Add(FString::Printf(TEXT("%s[%s]"), *PropertyName, *KeyValue));
					continue;
				}
				
				FStructProperty* StructValueProperty = CastField<FStructProperty>(ValueProperty);
				if (StructValueProperty)
				{
					TArray<FString> InnerNullPtrProperties;
					FindNullPtrProperties(ValueData, StructValueProperty->Struct, InnerNullPtrProperties);
					for (const FString& NullPtrProperty : InnerNullPtrProperties)
					{
						OutNullPtrProperties.Add(FString::Printf(TEXT("%s[%s].%s"), *PropertyName, *KeyValue, *NullPtrProperty));
					}
				}
			}
			continue;
		}

		FSetProperty* SetProperty = CastField<FSetProperty>(Property);
		if (SetProperty)
		{
			if (!ShouldSearchSetProperty(SetProperty))
			{
				continue;
			}
			
			FProperty* ElementProperty = SetProperty->ElementProp;
			FScriptSetHelper Helper(SetProperty, SetProperty->ContainerPtrToValuePtr<void>(FromContainer));
			for (int32 Index = 0; Index < Helper.Num(); ++Index)
			{
				if (!Helper.IsValidIndex(Index))
				{
					continue;
				}

				uint8* ElementData = Helper.GetElementPtr(Index);
				if (DoesPropertyContainNullPtrNoContainer(ElementProperty, ElementData))
				{
					OutNullPtrProperties.Add(FString::Printf(TEXT("%s[%d]"), *PropertyName, Index));
					continue;
				}

				FStructProperty* StructElementProperty = CastField<FStructProperty>(ElementProperty);
				if (StructElementProperty)
				{
					TArray<FString> InnerNullPtrProperties;
					FindNullPtrProperties(ElementData, StructElementProperty->Struct, InnerNullPtrProperties);
					for (const FString& NullPtrProperty : InnerNullPtrProperties)
					{
						OutNullPtrProperties.Add(FString::Printf(TEXT("%s[%d].%s"), *PropertyName, Index, *NullPtrProperty));
					}
				}
			}
			continue;
		}

		if (Property->HasMetaData(NullPtrMetaDataFlag))
		{
			if (DoesPropertyContainNullPtrNoContainer(Property, FromContainer))
			{
				OutNullPtrProperties.Add(PropertyName);
				continue;
			}
		}

		FStructProperty* StructInnerProperty = CastField<FStructProperty>(Property);
		if (StructInnerProperty)
		{
			void* StructData = StructInnerProperty->ContainerPtrToValuePtr<void>(FromContainer);
			TArray<FString> InnerNullPtrProperties;

			FindNullPtrProperties(StructData, StructInnerProperty->Struct, InnerNullPtrProperties);
			for (const FString& NullPtrProperty : InnerNullPtrProperties)
			{
				OutNullPtrProperties.Add(FString::Printf(TEXT("%s.%s"), *Property->GetName(), *NullPtrProperty));
			}
			continue;
		}
	}
}

bool UFindNullPtrsCommandlet::DoesPropertyContainNullPtrNoContainer(FProperty* Property, void* FromContainer)
{
	FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property);
	if (ObjectProperty)
	{
		UObject* Value = ObjectProperty->GetPropertyValue_InContainer(FromContainer);
		if (IsValid(Value))
		{
			return false;
		}

		return true;
	}

	FClassProperty* ClassProperty = CastField<FClassProperty>(Property);
	if (ClassProperty)
	{
		UObject* Value = ClassProperty->GetPropertyValue_InContainer(FromContainer);
		if (IsValid(Value))
		{
			return false;	
		}

		return true;
	}

	FSoftObjectProperty* SoftObjectProperty = CastField<FSoftObjectProperty>(Property);
	if (SoftObjectProperty)
	{
		FSoftObjectPtr Value = SoftObjectProperty->GetPropertyValue_InContainer(FromContainer);
		if (Value.IsValid())
		{
			return false;
		}

		return true;
	}

	FSoftClassProperty* SoftClassProperty = CastField<FSoftClassProperty>(Property);
	if (SoftClassProperty)
	{
		FSoftObjectPtr Value = SoftClassProperty->GetPropertyValue_InContainer(FromContainer);
		if (Value.IsValid())
		{
			return false;
		}

		return true;
	}
	
	return false;
}

bool UFindNullPtrsCommandlet::DoesArrayPropertyContainNullPtr(FProperty* ArrayInnerProperty, void* ArrayElementPtr)
{	
	FObjectProperty* ObjectProperty = CastField<FObjectProperty>(ArrayInnerProperty);
	if (ObjectProperty)
	{
		UObject* Value = *static_cast<UObject**>(ArrayElementPtr);
		if (IsValid(Value))
		{
			return false;
		}
		
		return true;
	}

	FClassProperty* ClassProperty = CastField<FClassProperty>(ArrayInnerProperty);
	if (ClassProperty)
	{
		UClass* Value = *static_cast<UClass**>(ArrayElementPtr);
		if (IsValid(Value))
		{
			return false;
		}
		
		return true;
	}

	FSoftObjectProperty* SoftObjectProperty = CastField<FSoftObjectProperty>(ArrayInnerProperty);
	if (SoftObjectProperty)
	{
		FSoftObjectPtr Value = SoftObjectProperty->GetPropertyValue_InContainer(ArrayElementPtr);
		if (Value.IsValid())
		{
			return false;
		}
		
		return true;
	}
	
	FSoftClassProperty* SoftClassProperty = CastField<FSoftClassProperty>(ArrayInnerProperty);
	if (SoftClassProperty)
	{
		FSoftObjectPtr Value = SoftClassProperty->GetPropertyValue_InContainer(ArrayElementPtr);
		if (Value.IsValid())
		{
			return false;
		}
		
		return true;
	}

	return false;
}

bool UFindNullPtrsCommandlet::ShouldSearchArrayProperty(FArrayProperty* ArrayProperty)
{
	if (ArrayProperty->Inner->IsA(FStructProperty::StaticClass()))
	{
		return true;
	}
	
	if (ArrayProperty->HasMetaData(NullPtrMetaDataFlag))
	{
		bool bIsPtr = IsPtrProperty(ArrayProperty->Inner);
		return bIsPtr;
	}

	return false;
}

bool UFindNullPtrsCommandlet::ShouldSearchMapProperty(FMapProperty* MapProperty)
{
	if (MapProperty->ValueProp->IsA(FStructProperty::StaticClass()))
	{
		return true;
	}

	if (MapProperty->HasMetaData(NullPtrMetaDataFlag))
	{
		bool bIsPtr = IsPtrProperty(MapProperty->ValueProp);
		return bIsPtr;
	}

	return false;
}

bool UFindNullPtrsCommandlet::ShouldSearchSetProperty(FSetProperty* SetProperty)
{
	if (SetProperty->ElementProp->IsA(FStructProperty::StaticClass()))
	{
		return true;
	}

	if (SetProperty->HasMetaData(NullPtrMetaDataFlag))
	{
		bool bIsPtr = IsPtrProperty(SetProperty->ElementProp);
		return bIsPtr;
	}

	return false;
}

bool UFindNullPtrsCommandlet::IsPtrProperty(FProperty* Property)
{
	return Property->IsA(FObjectProperty::StaticClass()) || Property->IsA(FClassProperty::StaticClass()) ||
		Property->IsA(FSoftObjectProperty::StaticClass()) || Property->IsA(FSoftClassProperty::StaticClass());
}