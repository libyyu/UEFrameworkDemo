// Copyright Epic Games, Inc. All Rights Reserved.

#include "FrameworkDemoGameMode.h"
#include "FrameworkDemoCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFrameworkDemoGameMode::AFrameworkDemoGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
