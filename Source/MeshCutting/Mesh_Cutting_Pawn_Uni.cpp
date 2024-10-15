// Fill out your copyright notice in the Description page of Project Settings.


#include "Mesh_Cutting_Pawn_Uni.h"

// Sets default values
AMesh_Cutting_Pawn_Uni::AMesh_Cutting_Pawn_Uni()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
// Create the Box Component
	box = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));

	//RootComponent = box;
	// Set the size of the box
	box->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));
	box->SetGenerateOverlapEvents(true);
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMesh_Cutting_Pawn_Uni::BeginPlay()
{
	FVector BoxExtent = box->GetScaledBoxExtent();

	Super::BeginPlay();
	
}

// Called every frame
void AMesh_Cutting_Pawn_Uni::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMesh_Cutting_Pawn_Uni::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("StartCutting", IE_Pressed, this, &AMesh_Cutting_Pawn_Uni::Start_Cutting);
	PlayerInputComponent->BindAction("StartCutting", IE_Released, this, &AMesh_Cutting_Pawn_Uni::Stop_Cutting);
}

void AMesh_Cutting_Pawn_Uni::Start_Cutting()
{
	box->SetCollisionProfileName(TEXT("OverlapAll"));

	// --LOOK INTO--
	////Might also need to update OverLaps
}

void AMesh_Cutting_Pawn_Uni::Stop_Cutting()
{
	UE_LOG(LogTemp, Warning, TEXT("Stop_Cutting called"));
	hitActorCutable = false;

	//if (isCutting) {
		//
		//DRAWING THE CUTOUT BOX IN DEBUG FOR TESTING - START
		//Drawing the debug box with the quat(Quaternion) for the rotation
	DrawDebugBox(GetWorld(), box->GetComponentLocation(), box->GetScaledBoxExtent(), box->GetComponentQuat(), FColor::Green, false, 3.0f, 0, 4);
	//DRAWING THE CUTOUT BOX IN DEBUG FOR TESTING - END
	//

//}
}