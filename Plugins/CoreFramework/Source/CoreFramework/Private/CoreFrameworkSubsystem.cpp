// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreFrameworkSubsystem.h"
#include "ResourceLoader.h"
#include "CoreFramework.h"

void UCoreFrameworkSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	UE_LOG(LogCoreFramework, Log, TEXT("UCoreFrameworkSubsystem::Initialize"));
}
	
void UCoreFrameworkSubsystem::Deinitialize()
{
	UE_LOG(LogCoreFramework, Log, TEXT("UCoreFrameworkSubsystem::Deinitialize"));
	ResourceLoader::Get().Shutdown();
}

bool UCoreFrameworkSubsystem::IsGameResLoaded(const FString& InName) const 
{
	return ResourceLoader::Get().IsGameResLoaded(InName);
}

void UCoreFrameworkSubsystem::AsyncLoadResource(const FString& InName, const TFunction<void(UObject*)>& InCallback, const FString& InObjType, bool IsBaseObjType)
{
	ResourceLoader::Get().AsyncLoadResource(InName, InCallback, InObjType, IsBaseObjType);
}

void UCoreFrameworkSubsystem::AsyncLoadResource(const FString& Name, FAsyncLoadResourceDelegate Event, const FString& ObjType, bool IsBaseObjType)
{
	AsyncLoadResource(Name, [Event](UObject* Obj)
	{
		Event.ExecuteIfBound(Obj);
	}, ObjType, IsBaseObjType);
}
	

UObject* UCoreFrameworkSubsystem::SyncLoadResource(const FString& InName, const FString& InObjType, bool IsBaseObjType)
{
	return ResourceLoader::Get().SyncLoadResource(InName, InObjType, IsBaseObjType);
}

void UCoreFrameworkSubsystem::Tick(float DeltaTime)
{
	ResourceLoader::Get().Tick();
}

