// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "ResourceLoaderSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class COREFRAMEWORK_API UResourceLoaderSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	virtual void Deinitialize() override;
protected:
	virtual bool IsTickable()const override { return true; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UResourceLoaderSubsystem, STATGROUP_Tickables); }
	virtual void Tick(float DeltaTime) override;
public:
	/*
	 *资源是否已加载好
	 */
	UFUNCTION(BlueprintCallable)
	bool IsGameResLoaded(const FString& InName) const;

	/*
	 *异步加载指定资源
	 */
	void AsyncLoadResource(const FString& InName, const TFunction<void(UObject*)>& InCallback, const FString& InObjType = TEXT(""), bool IsBaseObjType = false);
	DECLARE_DYNAMIC_DELEGATE_OneParam(FAsyncLoadResourceDelegate, UObject*, Obj);
	UFUNCTION(BlueprintCallable)
	void AsyncLoadResource(const FString& Name, FAsyncLoadResourceDelegate Event, const FString& ObjType = TEXT(""), bool IsBaseObjType = false);
	
	/*
	 *同步加载指定资源
	 */
	UFUNCTION(BlueprintCallable)
	UObject* SyncLoadResource(const FString& InName, const FString& InObjType = TEXT(""), bool IsBaseObjType = false);

};
