// Copyright Epic Games, Inc. All Rights Reserved.

#include "MeshCuttingGameMode.h"
#include "MeshCuttingCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMeshCuttingGameMode::AMeshCuttingGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
