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
	box->SetCollisionProfileName(TEXT("OverlapAll"));
	
}

// Called every frame
void AMesh_Cutting_Pawn_Uni::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (gateOpen) {

		SetUpCutting();
		
		//Visualisng DebugBox And Cut Area
	}
}

// Called to bind functionality to input
void AMesh_Cutting_Pawn_Uni::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	//if (pickedUp) {

	PlayerInputComponent->BindAction("StartCutting", IE_Pressed, this, &AMesh_Cutting_Pawn_Uni::Start_Cutting);
	PlayerInputComponent->BindAction("StartCutting", IE_Released, this, &AMesh_Cutting_Pawn_Uni::Stop_Cutting);
	//d}

}

void AMesh_Cutting_Pawn_Uni::Start_Cutting()
{
	gateOpen = true;
	//if (pickedUp) {
		box->SetCollisionProfileName(TEXT("OverlapAll"));
	//}
	// --LOOK INTO--
	////Might also need to update OverLaps
}

void AMesh_Cutting_Pawn_Uni::Stop_Cutting()
{
	gateOpen = false;

	//if (pickedUp) {

		UE_LOG(LogTemp, Warning, TEXT("Stop_Cutting called"));
		hitActorCutable = false;

		if (isCutting) {
			//
			//DRAWING THE CUTOUT BOX IN DEBUG FOR TESTING - START
			//Drawing the debug box with the quat(Quaternion) for the rotation
		DrawDebugBox(GetWorld(), box->GetComponentLocation(), box->GetScaledBoxExtent(), box->GetComponentQuat(), FColor::Green, false, 3.0f, 0, 4);
		//DRAWING THE CUTOUT BOX IN DEBUG FOR TESTING - END
		//

	}
	//}
}

void AMesh_Cutting_Pawn_Uni::SetUpCutting()
{
	//Setting up Variables

	FVector Start = GetActorLocation();
	FVector End = Start + GetActorForwardVector() * 5000;
	FHitResult hitResult;

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	if (GetWorld()->LineTraceSingleByChannel(hitResult, Start, End, ECC_Visibility, CollisionParams))
	{
		DrawDebugLine(GetWorld(), Start, hitResult.Location, FColor::Red, false, 0.2f, 0, 1.0f);
		// Check if something was hit
		if (hitResult.GetActor())
		{
			UE_LOG(LogTemp, Warning, TEXT("Hit: %s"), *hitResult.GetActor()->GetName());
			lastImpactPoint = hitResult.ImpactPoint;
			//getting the procedural mesh component from the actor
			UProceduralMeshComponent* ProcMeshComp = Cast<UProceduralMeshComponent>(hitResult.GetActor()->GetComponentByClass(UProceduralMeshComponent::StaticClass()));
			if (ProcMeshComp)//making sure there is a procedural mesh component
			{
				if (!isCutting) {
					isCutting = true;
					boxOrigin = hitResult.ImpactPoint;

					//Projecting the Vector onto the plane Variables
					FVector normalizedNormal = hitResult.ImpactNormal.GetSafeNormal();

					// Calculate the distance from the vector to the plane
					float distance = FVector::DotProduct(GetActorRightVector(), normalizedNormal);

					// Project the vector onto the plane
					FVector projectedVector = GetActorRightVector() - distance * normalizedNormal;


#pragma region Making the Rotation from Y to Z
					//Making the Rotation from Y to Z
					FVector normalizedY = projectedVector.GetSafeNormal();
					FVector normalizedZ = hitResult.ImpactNormal.GetSafeNormal();
					// Calculate the X vector using the cross product
					FVector xVector = FVector::CrossProduct(normalizedY, normalizedZ).GetSafeNormal();

					// Recalculate the Z vector to ensure it's orthogonal
					FVector correctedZ = FVector::CrossProduct(xVector, normalizedY).GetSafeNormal();

					// Create a rotation from the orthonormal basis
					FMatrix rotationMatrix = FMatrix(xVector, normalizedY, correctedZ, FVector::ZeroVector);
					FRotator rewRotation = rotationMatrix.Rotator();
#pragma endregion
					boxRotation = rewRotation;
					if (hitResult.GetActor()->FindComponentByClass<UProceduralMeshComponent>()) {
						hitActorCutable = true;
					}
					else {
						hitActorCutable = false;

					}
				}
			}
		}
	}
}

void AMesh_Cutting_Pawn_Uni::SetUpDebug()
{
	//NEXT PART (CURRENTLY WORKING ON THIS)
}

void AMesh_Cutting_Pawn_Uni::PickedUp(AActor* attachTo)
{
	this->AttachToActor(attachTo, FAttachmentTransformRules::KeepRelativeTransform);
		pickedUp = true;
}
