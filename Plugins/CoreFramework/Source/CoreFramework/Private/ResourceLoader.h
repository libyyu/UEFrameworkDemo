// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/Package.h"
#include "UObject/UObjectHash.h"

/*
 * 资源加载管理器
 */
class ResourceLoader : public FGCObject
{
	ResourceLoader();
	~ResourceLoader();

	void InnerLoadCallback(const FName& PackageName, UPackage* ResPackage, EAsyncLoadingResult::Type Result, FName realPckName);

	static void _AsyncLoadCallbackWrapper(const FName& PackageName, UPackage* ResPackage, EAsyncLoadingResult::Type Result, FName realPckName);

	typedef TFunction<void(TArray<UObject*>)> ResourceCallbackFuncType;
	typedef TArray<ResourceCallbackFuncType> CallbacksArray;
	TArray<CallbacksArray*> callbacksArrayCache;
	TMap<FName, CallbacksArray*> callbacksMap;

	CallbacksArray* GetOrCreateCallbacksArray()
	{
		if (callbacksArrayCache.Num() > 0)
			return callbacksArrayCache.Pop(false);

		return new CallbacksArray();
	}

	void AddCallbacksArrayToCache(CallbacksArray* callbacks)
	{
		if (callbacksArrayCache.Num() < 64)
		{
			callbacks->Reset();
			callbacksArrayCache.Add(callbacks);
		}
		else
		{
			delete callbacks;
		}
	}

	// Begin FGCObject
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	// End FGCObject

	struct DelayedCallbackInfo
	{
		FName PackageName;
		UPackage* ResPackage;
		EAsyncLoadingResult::Type Result;
		FName realPckName;
	};

	TArray<DelayedCallbackInfo> delayedCallbackInfos;
	TArray<DelayedCallbackInfo> tempCallbackInfos;
	TArray<UObject*> delayedCallbackRes;
	TArray<UObject*> tempCallbackRes;

public:

	static ResourceLoader& Get();

	/*
	 *异步加载资源包
	 */
	void LoadGameResAsync(const FString& InName, ResourceCallbackFuncType&& func, bool IsWorld = false);
	void LoadGameResAsync(const FString& InName, const ResourceCallbackFuncType& func, bool IsWorld = false)
	{
		ResourceCallbackFuncType tempFunc(func);
		LoadGameResAsync(InName, MoveTemp(tempFunc), IsWorld);
	}
	
	/*
	 *同步加载资源包
	 */
	void LoadGameRes(const FString& InName, TArray<UObject*>& res, bool IsWorld = false);

	/*
	 *资源是否已加载好
	 */
	bool IsGameResLoaded(const FString& InName);

	/*
	 *异步加载指定资源
	 */
	void AsyncLoadResource(const FString& InName, const TFunction<void(UObject*)>& InCallback, const FString& InObjType = TEXT(""), bool IsBaseObjType = false);
	/*
	 *同步加载指定资源
	 */
	UObject* SyncLoadResource(const FString& InName, const FString& InObjType = TEXT(""), bool IsBaseObjType = false);

public:
	void Tick();
	void Shutdown();
};
