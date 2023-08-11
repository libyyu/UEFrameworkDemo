// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "ResourceLoaderAsyncAction.generated.h"

/**
 * 
 */


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FResourceLoaderAsyncActionDelegate, UObject*, Obj);

UCLASS()
class COREFRAMEWORK_API UResourceLoaderAsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "ASync")
	static UResourceLoaderAsyncAction* NewResourceLoader(const FString& ResName, const FString& ObjType = TEXT(""), bool BaseObjType = false);
	
	UPROPERTY(BlueprintAssignable)
	FResourceLoaderAsyncActionDelegate OnComplete;

	virtual void Activate() override;
private:
	FString ResName;
	FString ObjType;
	bool IsBaseObjType = false;
};
