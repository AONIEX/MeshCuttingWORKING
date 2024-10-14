// Fill out your copyright notice in the Description page of Project Settings.


#include "Uni_CuttingMeshes_Character.h"

// Sets default values
AUni_CuttingMeshes_Character::AUni_CuttingMeshes_Character()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AUni_CuttingMeshes_Character::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AUni_CuttingMeshes_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AUni_CuttingMeshes_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

