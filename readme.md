Welcome to Ultimate Asset Validator!
Easy way to ensure your project has working data

## Getting Started
This plugin works using commandlets, for easier commandlet usage I recommend you using https://github.com/Flying-Rat/UE-Tool-Build-Helper.
Nontheless. Without it, it's not that hard to run commandlets on your own in your own project.
After including this plugin in your project this plugin brings you 2 commandlets:
- ValidateAssetsCommandlet - Every UObject in Unreal can override IsValid function which returns warnings and errors, this commandlet finds all assets in the specified directory and runs Unreal's standard validation workflow
    - Arguments:
        - AssetsDirectories - here you can specify all content directories you want validated. This is string array so separate like this "/Game/MyFolderInContent1,/Game/MyFolderInContent2". For example if you want all content directories to be validated just specify "/game/" (it's important for UE to include the first backslash / don't ask me why :D)
        - IncludeEngineContent - wheter this commandlet should also include engine content when running the validation, by default false
- FindNullPtrsCommandlet - Commandlet designed to help you find assets which depends on some other asset references but miss the reference. This is the ultimate commandlet helping you prevent all the crashes!
- code:
    - When making your code in the UPROPERTY specifier your can use meta to specify metadata we can read. NotNullPtr is the key one allowing us to detect an unreferenced asset. To prevent validation on your UPROPERTY you can use SkipUAV. So in practice your code could look like this
    UPROPERTY(meta = NotNullPtr)
    TSubclassOf<UObject> ImportantAssetReference;
    Like this we can find your assets and see if your variable does in fact have serialized a valid reference. Because this can go into nested structs, arrays and maps use UPROPERTY(meta = SkipUAV) if you want your specific (maybe nested struct or collection) variable to not be validated

## How to run a commandlet
Commandlets are run using commandline tools such as bash or powershell. you need to run an Unreal Engine executable found on path Engine/Binaries/Win64/ called UnrealEditor-Cmd.exe.
In order to trigger commandlets you need to use the following arguments:
- UProject path - find your .uproject file (named after your project typically) and use it as the first argument
- Commandlet name - Run="ModuleName.CommandletName" This is the format of specifying the commandlets. Remember that all commandlet scripts end with Commandlet word which should be specified
- Any additional arguments - you can run your commadlet with any arguments you want which it can read when ran

Commandlets are also debuggable with your IDE, just check your IDE run arguments and add to it -run="<moduleName.commandletName>" and any arguments you want