// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceLoaderSubsystem.h"
#include "ResourceLoader.h"
#include "CoreFramework.h"

void UResourceLoaderSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	UE_LOG(LogCoreFramework, Log, TEXT("ResourceLoaderSubsystem::Initialize"));
}
	
void UResourceLoaderSubsystem::Deinitialize()
{
	UE_LOG(LogCoreFramework, Log, TEXT("ResourceLoaderSubsystem::Deinitialize"));
	ResourceLoader::Get().Shutdown();
}

bool UResourceLoaderSubsystem::IsGameResLoaded(const FString& InName)
{
	return ResourceLoader::Get().IsGameResLoaded(InName);
}

void UResourceLoaderSubsystem::AsyncLoadResource(const FString& InName, const TFunction<void(UObject*)>& InCallback, const FString& InObjType, bool IsBaseObjType)
{
	ResourceLoader::Get().AsyncLoadResource(InName, InCallback, InObjType, IsBaseObjType);
}

UObject* UResourceLoaderSubsystem::SyncLoadResource(const FString& InName, const FString& InObjType, bool IsBaseObjType)
{
	return ResourceLoader::Get().SyncLoadResource(InName, InObjType, IsBaseObjType);
}

void UResourceLoaderSubsystem::Tick(float DeltaTime)
{
	ResourceLoader::Get().Tick();
}

