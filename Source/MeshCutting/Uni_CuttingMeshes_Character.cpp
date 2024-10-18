// Fill out your copyright notice in the Description page of Project Settings.


#include "Uni_CuttingMeshes_Character.h"




// Sets default values
AUni_CuttingMeshes_Character::AUni_CuttingMeshes_Character()
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
void AUni_CuttingMeshes_Character::BeginPlay()
{
	FVector BoxExtent = box->GetScaledBoxExtent();

	Super::BeginPlay();
	box->SetCollisionProfileName(TEXT("OverlapAll"));

}

// Called every frame
void AUni_CuttingMeshes_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (gateOpen) {

		SetUpCutting();

		//Visualisng DebugBox And Cut Area
	}
}

// Called to bind functionality to input
void AUni_CuttingMeshes_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	//if (pickedUp) {

	PlayerInputComponent->BindAction("StartCutting", IE_Pressed, this, &AUni_CuttingMeshes_Character::Start_Cutting);
	PlayerInputComponent->BindAction("StartCutting", IE_Released, this, &AUni_CuttingMeshes_Character::Stop_Cutting);
	//d}

}

void AUni_CuttingMeshes_Character::Start_Cutting()
{
	gateOpen = true;
	//if (pickedUp) {
	box->SetCollisionProfileName(TEXT("OverlapAll"));
	//}
	// --LOOK INTO--
	////Might also need to update OverLaps
}

void AUni_CuttingMeshes_Character::Stop_Cutting()
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

void AUni_CuttingMeshes_Character::SetUpCutting()
{
	//Setting up Variables
	UCameraComponent* cameraComponent = this->FindComponentByClass<UCameraComponent>();

	//ForTesting
	FVector start = GetActorLocation();
	//For Final Use 
	//FVector Start = cameraComponent->GetComponentLocation();
	FVector end = start + cameraComponent->GetForwardVector() * 5000;
	FHitResult hitResult;
	FCollisionQueryParams collisionParams;
	collisionParams.AddIgnoredActor(this);
	if (GetWorld()->LineTraceSingleByChannel(hitResult, start, end, ECC_Visibility, collisionParams))
	{
		DrawDebugLine(GetWorld(), start, hitResult.Location, FColor::Red, false, 0.2f, 0, 1.0f);
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

void AUni_CuttingMeshes_Character::SetUpDebug()
{
	//NEXT PART (CURRENTLY WORKING ON THIS)
}

void AUni_CuttingMeshes_Character::PickedUp(AActor* attachTo)
{
	this->AttachToActor(attachTo, FAttachmentTransformRules::KeepRelativeTransform);
	pickedUp = true;
}
