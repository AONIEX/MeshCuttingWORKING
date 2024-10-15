// Fill out your copyright notice in the Description page of Project Settings.


#include "Uni_CuttingMeshes_Character.h"

// Sets default values
AUni_CuttingMeshes_Character::AUni_CuttingMeshes_Character()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	// Create the Box Component
	box = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	UCapsuleComponent* capsuleComp = GetCapsuleComponent();
	
	//RootComponent = box;
	// Set the size of the box
	box->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));
	box->SetGenerateOverlapEvents(true);

	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AUni_CuttingMeshes_Character::BeginPlay()
{
	FVector BoxExtent = box->GetScaledBoxExtent();
	Super::BeginPlay();
	
	
}

// Called every frame
void AUni_CuttingMeshes_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//Stop_Cutting();
}

// Called to bind functionality to input
void AUni_CuttingMeshes_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	/*PlayerInputComponent->BindAction("StartCutting", IE_Pressed, this, &AUni_CuttingMeshes_Character::Start_Cutting);
	PlayerInputComponent->BindAction("StartCutting", IE_Released, this, &AUni_CuttingMeshes_Character::Stop_Cutting);*/
}

void AUni_CuttingMeshes_Character::Start_Cutting()
{
	box->SetCollisionProfileName(TEXT("OverlapAll"));

	// --LOOK INTO--
	////Might also need to update OverLaps
}

void AUni_CuttingMeshes_Character::Stop_Cutting()
{
	UE_LOG(LogTemp, Warning, TEXT("Stop_Cutting called"));
	hitActorCutable = false;
	
	//if (isCutting) {
		//
		//DRAWING THE CUTOUT BOX IN DEBUG FOR TESTING - START
		//Drawing the debug box with the quat(Quaternion) for the rotation
		DrawDebugBox(GetWorld(), box->GetComponentLocation() , box->GetScaledBoxExtent(), box->GetComponentQuat(), FColor::Green, true, 3.0f, 0, 4);
		//DRAWING THE CUTOUT BOX IN DEBUG FOR TESTING - END
		//

	//}
}

