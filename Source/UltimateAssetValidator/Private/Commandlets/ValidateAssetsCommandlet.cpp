// Copyright 2022 Tadeáš Anděl, All Rights Reserved.


#include "Commandlets/ValidateAssetsCommandlet.h"

#include "EditorValidatorSubsystem.h"
#include "UltimateAssetValidator.h"

#include "AssetRegistry/AssetRegistryModule.h"

UValidateAssetsCommandlet::UValidateAssetsCommandlet()
{
	FastExit = true;
	LogToConsole = true;
}

int32 UValidateAssetsCommandlet::Main(const FString& FullCommandLine)
{
	UE_LOG(LogInit, Log, TEXT("Validating assets"));
	
	TArray<FString> Tokens;
	TArray<FString> Switches;
	TMap<FString, FString> Params;
	ParseCommandLine(*FullCommandLine, Tokens, Switches, Params);

	UEditorValidatorSubsystem* EditorValidatorSubsystem = GEditor->GetEditorSubsystem<UEditorValidatorSubsystem>();
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(AssetRegistryConstants::ModuleName);
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Get all assets in the project
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
		AssetRegistry.GetAllAssets(AllAssets);
	}

	if (!Switches.Contains(TEXT("bIncludeEngine")))
	{
		FString EngineDir = FPaths::ConvertRelativePathToFull(FPaths::EngineDir());
		AllAssets.RemoveAll([&EngineDir](const FAssetData& AssetData)
			{
				// Remove /Engine and any plugins from /Engine, but keep /Game and any plugins under /Game.
				FString FileName;
				FString PackageName;
				AssetData.PackageName.ToString(PackageName);
				if (!FPackageName::TryConvertLongPackageNameToFilename(PackageName, FileName))
				{
					// We don't recognize this packagepath, so keep it
					return false;
				}
				// ConvertLongPackageNameToFilename can return ../../Plugins for some plugins instead of
				// ../../../Engine/Plugins. We should fix that in FPackageName to always return the normalized
				// filename. For now, workaround it by converting to absolute paths.
				FileName = FPaths::ConvertRelativePathToFull(MoveTemp(FileName));
				return FPathViews::IsParentPathOf(EngineDir, FileName);
			});
	}
	
	FValidateAssetsSettings Settings;
	
	Settings.bSkipExcludedDirectories = true;
	Settings.bShowIfNoFailures = true;
	Settings.bCollectPerAssetDetails = true;
	Settings.bLoadAssetsForValidation = true;
	Settings.ValidationUsecase = EDataValidationUsecase::Commandlet;
	FValidateAssetsResults Results;
	const int32 Ret = EditorValidatorSubsystem->ValidateAssetsWithSettings(AllAssets, Settings, Results);

	FString InvalidFiles;
	FString ValidFiles;
	FString NotValidatedFiles;
	for (const TTuple<FString, FValidateAssetsDetails>& AssetsDetail : Results.AssetsDetails) 
	{
		switch (AssetsDetail.Value.Result)
		{
			case EDataValidationResult::Invalid:
			{
				FString ValidationErrorsText;
				for (const FText& ValidationError : AssetsDetail.Value.ValidationErrors)
				{
					ValidationErrorsText += ValidationError.ToString() + TEXT("\n");
				}

				FString ValidationWarningsText;
				for (const FText& ValidationWarning : AssetsDetail.Value.ValidationWarnings)
				{
					ValidationWarningsText += ValidationWarning.ToString() + TEXT("\n");
				}
				FString ErrorString = FString::Printf(TEXT("Invalid asset %s with errors: \n%s and with warnings: %s"), *AssetsDetail.Key, *ValidationErrorsText, *ValidationWarningsText);
				UE_LOG(LogUAV, Error, TEXT("%s"), *ErrorString);
				InvalidFiles += ErrorString + TEXT("\n");
				break;
			}
			case EDataValidationResult::Valid:
			{
				if (AssetsDetail.Value.ValidationWarnings.Num() > 0)
				{
					FString ValidationWarningsText;
					for (const FText& ValidationWarning : AssetsDetail.Value.ValidationWarnings)
					{
						ValidationWarningsText += ValidationWarning.ToString() + TEXT("\n");
					}

					FString WarningString = FString::Printf(TEXT("Valid asset %s with warnings: \n%s"), *AssetsDetail.Key, *ValidationWarningsText);
					UE_LOG(LogUAV, Warning, TEXT("%s"), *WarningString);
					ValidFiles += WarningString + TEXT("\n");
				}
				break;
			}
			case EDataValidationResult::NotValidated:
			{
				FString ErrorString = FString::Printf(TEXT("Unable to validate asset %s\n"), *AssetsDetail.Key);
				UE_LOG(LogUAV, Error, TEXT("%s"), *ErrorString);
				NotValidatedFiles += ErrorString;
				break;
			}
		}
	}

	if (InvalidFiles.Len() > 0)
	{
		FString FileName = FPaths::Combine(FPaths::ProjectSavedDir(), "Logs", "InvalidAssets.txt");
		FFileHelper::SaveStringToFile(InvalidFiles, *FileName);
	}

	if (ValidFiles.Len() > 0)
	{
		FString FileName = FPaths::Combine(FPaths::ProjectSavedDir(), "Logs", "ValidAssets.txt");
		FFileHelper::SaveStringToFile(ValidFiles, *FileName);
	}

	if (NotValidatedFiles.Len() > 0)
	{
		FString FileName = FPaths::Combine(FPaths::ProjectSavedDir(), "Logs", "NotValidatedAssets.txt");
		FFileHelper::SaveStringToFile(NotValidatedFiles, *FileName);
	}
	
	return Ret;
}