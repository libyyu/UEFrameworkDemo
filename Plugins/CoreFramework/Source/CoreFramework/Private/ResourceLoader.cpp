// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceLoader.h"
#include "CoreFramework.h"
#include "Misc/EngineVersionComparison.h"

static bool ConvertResNameToPackagePath(const FString& InName, FString& OutPackagePath, FString& OutErrorInfo)
{
	if (InName.IsEmpty())
	{
		OutErrorInfo = TEXT("empty res name");
		return false;
	}

	if (InName.EndsWith(TEXT("/"), ESearchCase::Type::CaseSensitive))
	{
		OutErrorInfo = FString::Printf(TEXT("resName[%s] EndsWith '/'!"), *InName);
		return false;
	}

	// 如果文件名含有'.'，则是非法的文件名，会崩溃
	int idx;
	if (InName.FindLastChar(TCHAR('.'), idx))
	{
		OutErrorInfo = FString::Printf(TEXT("resName[%s] contains '.'!"), *InName);
		return false;
	}

	const int maxNameLen = 500;
	int inNameLen = InName.Len();

	if (inNameLen > maxNameLen)
	{
		OutErrorInfo = FString::Printf(TEXT("resName[%s] too long, max: %d, got: %d"), *InName, maxNameLen, inNameLen);
		return false;
	}

	const TCHAR* prefixGame = TEXT("/Game/");
	const int prefixLen = FCString::Strlen(prefixGame);
	TCHAR tempPath[512];
	FCString::Strncpy(tempPath, prefixGame, prefixLen + 1);
	FCString::Strncpy(tempPath + prefixLen, *InName, inNameLen + 1);
	OutPackagePath.Reset(0);
	OutPackagePath.Reserve(maxNameLen);
	OutPackagePath.AppendChars(tempPath, prefixLen + inNameLen);
	return true;
}

#if UE_BUILD_DEBUG
static void _LoadGameResFromPck(const UPackage* ResPackage, TArray<UObject*>& PotentialRes)
#else
FORCEINLINE static void _LoadGameResFromPck(const UPackage* ResPackage, TArray<UObject*>& PotentialRes)
#endif
{
	//GetObjectsWithPackage(ResPackage, PotentialRes, false);
	GetObjectsWithOuter(ResPackage, PotentialRes, false);
#if UE_BUILD_DEBUG
	if (PotentialRes.Num() == 0)
		check(true);
#endif
}
	
ResourceLoader::ResourceLoader()
{
}

ResourceLoader::~ResourceLoader()
{
	for (auto cache : callbacksArrayCache)
		delete cache;

	for (auto& cb : callbacksMap)
		delete cb.Value;
}

ResourceLoader& ResourceLoader::Get()
{
	static ResourceLoader inst;
	return inst;
}

void ResourceLoader::InnerLoadCallback(const FName& PackageName, const UPackage* ResPackage, EAsyncLoadingResult::Type Result, FName RealPckName)
{
	TArray<UObject*> PotentialRes;
	if (Result == EAsyncLoadingResult::Succeeded && ResPackage && ResPackage->IsValidLowLevel())
		_LoadGameResFromPck(ResPackage, PotentialRes);

	auto& callbacksMap2 = callbacksMap;
	CallbacksArray* callbacksPtr = *(callbacksMap2.Find(RealPckName));
	callbacksMap2.Remove(RealPckName);

	for (auto& oneFunc : *callbacksPtr)
	{
		oneFunc(PotentialRes);
	}

	AddCallbacksArrayToCache(callbacksPtr);
}
	
void ResourceLoader::_AsyncLoadCallbackWrapper(const FName& PackageName, UPackage* ResPackage, EAsyncLoadingResult::Type Result, FName RealPckName)
{
	//if (GAzureFlushAsyncLoadingFlag == 0)
	//{
	//	Get().InnerLoadCallback(PackageName, ResPackage, Result, realPckName);
	//	return;
	//}
	
	auto& inst = Get();
	inst.delayedCallbackInfos.Add({ PackageName, ResPackage, Result, RealPckName });
	if (ResPackage)
	{
		//GetObjectsWithPackage(ResPackage, inst.delayedCallbackRes, false);
		GetObjectsWithOuter(ResPackage, inst.delayedCallbackRes, false);
	}
}

void ResourceLoader::LoadGameResAsync(const FString& InName, ResourceCallbackFuncType&& Func, bool IsWorld)
{
	static FString PckPath;
	FString ErrorInfo;
	if (!ConvertResNameToPackagePath(InName, PckPath, ErrorInfo))
	{
		UE_LOG(LogCoreFramework, Warning, TEXT("LoadGameResAsync failed %s, %s"), *PckPath, *ErrorInfo);
		Func(TArray<UObject*>());
		return;
	}
	
	const UPackage* PackagePtr = FindPackage(nullptr, *PckPath);

	if (PackagePtr && PackagePtr->IsFullyLoaded())
	{
		TArray<UObject*> PotentialRes;
		 _LoadGameResFromPck(PackagePtr, PotentialRes);
		Func(PotentialRes);
	}
	else
	{
		FName PckName(*PckPath);
		CallbacksArray** callbacksPtr = callbacksMap.Find(PckName);

		if (callbacksPtr == nullptr)
		{
			CallbacksArray* callbacks = GetOrCreateCallbacksArray();
			callbacks->Add(MoveTemp(Func));
			callbacksMap.Add(PckName, callbacks);

			EPackageFlags PackageFlags = IsWorld ? PKG_ContainsMap : PKG_None;
			int32 PIEInstanceID = INDEX_NONE;

#if WITH_EDITOR
			if (IsWorld)
			{
				UWorld* PersistentWorld = GWorld;
				if (PersistentWorld && PersistentWorld->IsPlayInEditor())
				{
					if (PersistentWorld->GetOutermost()->HasAnyPackageFlags(PKG_PlayInEditor))
					{
						PackageFlags |= PKG_PlayInEditor;
					}
#if UE_VERSION_OLDER_THAN(5, 0, 0)
					PIEInstanceID = PersistentWorld->GetOutermost()->PIEInstanceID;
#else
					PIEInstanceID = PersistentWorld->GetOutermost()->GetPIEInstanceID();
#endif
				}
			}
#endif

#if UE_VERSION_OLDER_THAN(5, 0, 0)
			LoadPackageAsync(PckPath, nullptr, nullptr, FLoadPackageAsyncDelegate::CreateStatic(&_AsyncLoadCallbackWrapper, PckName), PackageFlags, PIEInstanceID, 0);
#else
			LoadPackageAsync(PckPath, FLoadPackageAsyncDelegate::CreateStatic(&_AsyncLoadCallbackWrapper, PckName), 0, PackageFlags, PIEInstanceID);
#endif
		}
		else
		{
			CallbacksArray* callbacks = *callbacksPtr;
			callbacks->Add(MoveTemp(Func));
		}
	}
}

void ResourceLoader::LoadGameRes(const FString& InName, TArray<UObject*>& Res, bool IsWorld)
{
	static FString PckPath;
	FString ErrorInfo;
	if (!ConvertResNameToPackagePath(InName, PckPath, ErrorInfo))
	{
		UE_LOG(LogCoreFramework, Warning, TEXT("LoadGameRes %s, %s"), *PckPath, *ErrorInfo);
		return;
	}
	
	const UPackage* ResPackage = FindPackage(nullptr, *PckPath);

	if (ResPackage && ResPackage->IsFullyLoaded())
	{
		_LoadGameResFromPck(ResPackage, Res);
		return;
	}

	if (IsAsyncLoading() && IsAsyncLoadingSuspended())
	{
		UE_LOG(LogCoreFramework, Error, TEXT("LoadGameRes, resName[%s] IsAsyncLoadingSuspended!"), *InName);
		return;
	}
	
	ResPackage = LoadPackage(nullptr, *PckPath, IsWorld ? LOAD_PackageForPIE : LOAD_None, nullptr);

	if (ResPackage == nullptr)
		return;

	_LoadGameResFromPck(ResPackage, Res);
}

bool ResourceLoader::IsGameResLoaded(const FString& InName) const
{
	static FString PckPath;
	FString ErrorInfo;
	if (!ConvertResNameToPackagePath(InName, PckPath, ErrorInfo))
	{
		return false;
	}

	const UPackage* ResPackage = FindPackage(nullptr, *PckPath);
	if (ResPackage && ResPackage->IsFullyLoaded())
	{
		return true;
	}

	return false;
}

void ResourceLoader::AddReferencedObjects(FReferenceCollector& Collector)
{
	for (auto& info : delayedCallbackInfos)
	{
		if (info.ResPackage)
			Collector.AddReferencedObject(info.ResPackage);
	}
	for (auto& info : tempCallbackInfos)
	{
		if (info.ResPackage)
			Collector.AddReferencedObject(info.ResPackage);
	}
	for (auto res : delayedCallbackRes)
		Collector.AddReferencedObject(res);
	for (auto res : tempCallbackRes)
		Collector.AddReferencedObject(res);
}

void ResourceLoader::Tick()
{
	if (delayedCallbackInfos.Num() == 0)
		return;

	tempCallbackInfos = delayedCallbackInfos;
	delayedCallbackInfos.Empty();
	tempCallbackRes = delayedCallbackRes;
	delayedCallbackRes.Empty();
	for (auto& info : tempCallbackInfos)
	{
		InnerLoadCallback(info.PackageName, info.ResPackage, info.Result, info.RealPckName);
	}

	tempCallbackInfos.Empty();
	tempCallbackRes.Empty();
}

void ResourceLoader::Shutdown()
{
	while (delayedCallbackInfos.Num() != 0)
	{
		Tick();
	}
}

void ResourceLoader::AsyncLoadResource(const FString& InName, const TFunction<void(UObject*)>& InCallback, const FString& InObjType, bool IsBaseObjType)
{
	bool isWorld = false;
	if (InObjType == TEXT("World"))
	{
		isWorld = true;
	}

	auto onFinish = [InName, InObjType, IsBaseObjType, InCallback](const TArray<UObject*>& res)
	{
		UObject* asset = nullptr;

		if (!InObjType.IsEmpty())
		{
			bool bFound = false;
			for (auto ObjIt = res.CreateConstIterator(); ObjIt; ++ObjIt)
			{
				UObject* ob = (*ObjIt);
				UClass* cls = ob->GetClass();

				while (cls)
				{
					if (cls->GetName() == InObjType)
					{
						asset = ob;
						bFound = true;
						break;
					}
					if (!IsBaseObjType)
						break;
					cls = cls->GetSuperClass();
				}
				if (bFound)
					break;
			}
		}
		else if (res.Num() == 1)
		{
			asset = res[0];
		}
		else if (res.Num())
		{
			int32 Index = 0;
			if (InName.FindLastChar(TEXT('/'), Index))
			{
				FString ObjName = InName.RightChop(Index + 1);
				for (const auto& Obj : res)
				{
					if (Obj->GetName() == ObjName)
					{
						asset = Obj;
						break;
					}
				}
			}
		}

		if(!asset)
		{
			UE_LOG(LogCoreFramework, Warning, TEXT("AsyncLoadResource failed to load: %s, type:%s"), *InName, *InObjType);
		}

		InCallback(asset);
	};

	LoadGameResAsync(InName, onFinish, isWorld);
}

UObject* ResourceLoader::SyncLoadResource(const FString& InName, const FString& InObjType, bool IsBaseObjType)
{
	bool isWorld = false;
	if (InObjType == TEXT("World"))
	{
		isWorld = true;
	}

	TArray<UObject*> res;
	LoadGameRes(InName, res, isWorld);
	UObject* asset = nullptr;

	if (!InObjType.IsEmpty())
	{
		for (auto ObjIt = res.CreateConstIterator(); ObjIt; ++ObjIt)
		{
			if ((*ObjIt)->GetClass()->GetName() == InObjType)
			{
				asset = (*ObjIt);
				break;
			}
		}
	}
	else if (res.Num() == 1)
	{
		asset = res[0];
	}
	else if (res.Num() > 0)
	{
		int32 Index = 0;
		if (InName.FindLastChar(TEXT('/'), Index))
		{
			FString ObjName = InName.RightChop(Index + 1);
			for (const auto& Obj : res)
			{
				if (Obj->GetName() == ObjName)
				{
					asset = Obj;
					break;
				}
			}
		}
	}

	if(!asset)
	{
		UE_LOG(LogCoreFramework, Warning, TEXT("SyncLoadResource failed to load: %s, type:%s"), *InName, *InObjType);
	}

	return asset;
}
