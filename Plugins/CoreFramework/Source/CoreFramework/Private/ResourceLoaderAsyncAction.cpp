// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceLoaderAsyncAction.h"
#include "ResourceLoaderSubsystem.h"
#include "Kismet/GameplayStatics.h"

UResourceLoaderAsyncAction* UResourceLoaderAsyncAction::NewResourceLoader(const FString& InName, const FString& InObjType, bool IsBaseObjType)
{
	UResourceLoaderAsyncAction* Obj = NewObject<UResourceLoaderAsyncAction>();
	checkf(Obj!=nullptr, TEXT("UResourceLoaderAsyncAction New Failed"));
	Obj->ResName = InName;
	Obj->ObjType = InObjType;
	Obj->IsBaseObjType = IsBaseObjType;
	Obj->AddToRoot();
	return Obj;
}

void UResourceLoaderAsyncAction::Activate()
{
	Super::Activate();
	
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld());
	UResourceLoaderSubsystem* inst = GameInstance->GetSubsystem<UResourceLoaderSubsystem>();
	inst->AsyncLoadResource(ResName, [this](UObject* Obj)
	{
		if(OnComplete.IsBound())
		{
			OnComplete.Broadcast(Obj);
		}
		RemoveFromRoot();
	}, ObjType, IsBaseObjType);
}

