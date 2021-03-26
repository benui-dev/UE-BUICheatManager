#pragma once

#include "GameFramework/CheatManager.h"
#include <ConsoleSettings.h>
#include "BUICheatManagerBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN( LogCheatManager, Log, All );

UCLASS()
class BUICHEATMANAGER_API UBUICheatManagerBase : public UCheatManager
{
	GENERATED_BODY()

public:
	virtual void InitCheatManager() override;
	virtual void BeginDestroy() override;
	virtual bool ProcessConsoleExec( const TCHAR* Cmd, FOutputDevice& Ar, UObject* Executor ) override;
	virtual void GetAssetRegistryTags( TArray<FAssetRegistryTag>& OutTags ) const override;

	FColor CheatColor = FColor::Cyan;
	FName CheatPrefixMetaKey = "CheatPrefix";
	FName CheatNameMetaKey = TEXT( "Cheat" );

private:
	void RegisterAutoCompleteEntries( TArray<FAutoCompleteCommand>& Commands ) const;
	bool GetAssetData( const UClass* ForClass, FAssetData& AssetData ) const;

};

