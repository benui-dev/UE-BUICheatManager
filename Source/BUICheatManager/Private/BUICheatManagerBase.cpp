#include "BUICheatManagerBase.h"

#include <Engine/Console.h>
#include <Engine/UserInterfaceSettings.h>
#include "AssetRegistry/AssetRegistryModule.h"

DEFINE_LOG_CATEGORY( LogCheatManager );

static int32 NumAutoCompletesRegistered = 0;
static const FString TagNamespace = TEXT( "CheatFunc:" );

void UBUICheatManagerBase::InitCheatManager()
{
	Super::InitCheatManager();

	if ( NumAutoCompletesRegistered <= 0 )
	{
		UE_LOG( LogCheatManager, Verbose, TEXT( "Registering cheat manager for autocomplete callbacks: %s" ), *GetName() );
		UConsole::RegisterConsoleAutoCompleteEntries.AddUObject( this, &UBUICheatManagerBase::RegisterAutoCompleteEntries );
		NumAutoCompletesRegistered += 1;
	}
	else
	{
		UE_LOG( LogCheatManager, Verbose, TEXT( "Didn't need to register cheat manager for autocomplete callbacks: %s" ), *GetName() );
	}
}

void UBUICheatManagerBase::BeginDestroy()
{
	int32 NumRemoved = UConsole::RegisterConsoleAutoCompleteEntries.RemoveAll( this );
	NumAutoCompletesRegistered -= NumRemoved;
	Super::BeginDestroy();
}


bool UBUICheatManagerBase::ProcessConsoleExec( const TCHAR* Cmd, FOutputDevice& Ar, UObject* Executor )
{
	FString OriginalCmd = Cmd;

	FString CommandName;
	if ( FParse::Token( Cmd, CommandName, true ) )
	{
		FAssetData Asset;
		if ( GetAssetData( GetClass(), Asset ) )
		{
			const FName CommandNameTag( TagNamespace + CommandName );
			FAssetDataTagMapSharedView::FFindTagResult Result = Asset.TagsAndValues.FindTag( CommandNameTag );
			if ( Result.IsSet() )
			{
				// Cmd will now just be the arguments since we parsed the command name out of it
				const FString CmdString = Result.GetValue() + Cmd;
				return CallFunctionByNameWithArguments( *CmdString, Ar, Executor, /* bForceCallWithNonExec */ true );
			}
		}
	}

	return CallFunctionByNameWithArguments( *OriginalCmd, Ar, Executor );
}

void UBUICheatManagerBase::GetAssetRegistryTags( TArray<FAssetRegistryTag>& OutTags ) const
{
	Super::GetAssetRegistryTags( OutTags );

#if WITH_EDITOR
	// Get all classes to search
	TArray<const UClass*> ClassesToSearch;
	const UClass* CurrentClass = GetClass();
	while ( CurrentClass != UBUICheatManagerBase::StaticClass() )
	{
		ClassesToSearch.Add( CurrentClass );
		CurrentClass = CurrentClass->GetSuperClass();
	}

	for ( const UClass* ClassToSearch : ClassesToSearch )
	{
		// Grab the overall class prefix
		const FString ClassCheatPrefix = ClassToSearch->GetMetaData( CheatPrefixMetaKey );

		// Find any functions with the Cheat meta (haven't bothered doing any duplicate detection or other validation)
		TArray<FName> FunctionNames;
		ClassToSearch->GenerateFunctionList( FunctionNames );
		for ( const FName& Name : FunctionNames )
		{
			const UFunction* Func = ClassToSearch->FindFunctionByName( Name );

			if ( !Func || !Func->HasMetaData( CheatNameMetaKey ) )
			{
				continue;
			}

			FString CheatName = Func->GetMetaData( CheatNameMetaKey );

			// If they didn't provide a custom cheat name, then just default to the function name
			if ( CheatName.IsEmpty() )
			{
				CheatName = Name.ToString();
			}

			// Prefix our tags with a namespace to help avoid conflicts
			FString CommandNameTag = TagNamespace + ClassCheatPrefix + CheatName;
			FAssetRegistryTag CommandTag( FName( CommandNameTag ), Name.ToString(), FAssetRegistryTag::TT_Hidden );
			OutTags.Add( CommandTag );
		}
	}
#endif
}

void UBUICheatManagerBase::RegisterAutoCompleteEntries( TArray<FAutoCompleteCommand>& Commands ) const
{
	TArray<const UClass*> ClassesToSearch;
	const UClass* CurrentClass = GetClass();
	while ( CurrentClass != UBUICheatManagerBase::StaticClass() )
	{
		ClassesToSearch.Add( CurrentClass );
		CurrentClass = CurrentClass->GetSuperClass();
	}

	for ( const UClass* ClassToSearch : ClassesToSearch )
	{
		FAssetData Asset;
		if ( !GetAssetData( ClassToSearch, Asset ) )
		{
			continue;
		}

		for ( const auto& Pair : Asset.TagsAndValues.CopyMap() )
		{
			const FString TagName = Pair.Key.ToString();
			if ( !TagName.StartsWith( TagNamespace ) )
			{
				continue;
			}

			const FName FunctionName( Pair.Value );
			const UFunction* Func = ClassToSearch->FindFunctionByName( FunctionName );
			if ( Func == nullptr )
			{
				continue;
			}

			// Build help text for each param (matches what the default autocomplete does in UConsole::BuildRuntimeAutoCompleteList)
			FString Desc;
			for ( TFieldIterator<FProperty> PropIt( Func ); PropIt && ( PropIt->PropertyFlags & CPF_Parm ); ++PropIt )
			{
				FProperty* Prop = *PropIt;
				Desc += FString::Printf( TEXT( "%s[%s] " ), *Prop->GetName(), *Prop->GetCPPType() );
			}

			FAutoCompleteCommand Entry;
			Entry.Command = TagName.Mid( TagNamespace.Len() );
			Entry.Desc = Desc;
			Entry.Color = CheatColor;

			Commands.Add( Entry );
		}
	}
}


bool UBUICheatManagerBase::GetAssetData( const UClass* ForClass, FAssetData& AssetData ) const
{
	const FString PackagePath = FPackageName::ObjectPathToPackageName( ForClass->GetPathName() );
	TArray<FAssetData> Assets;
	FAssetRegistryModule::GetRegistry().GetAssetsByPackageName( FName( PackagePath ), Assets );

	if ( Assets.Num() == 0 )
	{
		UE_LOG( LogCheatManager, Warning, TEXT( "Could not find any asset data for package %s, perhaps this isn't a Blueprint subclass?" ), *PackagePath );
		return false;
	}

	AssetData = Assets[ 0 ];
	return true;
}

