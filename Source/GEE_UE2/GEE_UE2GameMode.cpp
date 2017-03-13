// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "GEE_UE2.h"
#include "GEE_UE2GameMode.h"
#include "GEE_UE2Character.h"

AGEE_UE2GameMode::AGEE_UE2GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
