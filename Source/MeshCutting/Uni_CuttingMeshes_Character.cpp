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
	box->SetBoxExtent(FVector(32.0f, 32.0f, 32.0f));
	box->SetGenerateOverlapEvents(true);
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AUni_CuttingMeshes_Character::BeginPlay()
{

	Super::BeginPlay();
	box->SetCollisionProfileName(TEXT("OverlapAll"));

}

// Called every frame
void AUni_CuttingMeshes_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (m_gateOpen) {

		SetUpCutting();
		SetUpDebug();
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
	m_gateOpen = true;
	//if (pickedUp) {
	box->SetCollisionProfileName(TEXT("OverlapAll"));
	//}
	// --LOOK INTO--
	////Might also need to update OverLaps
}

void AUni_CuttingMeshes_Character::Stop_Cutting()
{
	m_gateOpen = false;

	//if (pickedUp) {

	UE_LOG(LogTemp, Warning, TEXT("Stop_Cutting called"));
	m_hitActorCutable = false;

	if (m_isCutting) {
		//
		//DRAWING THE CUTOUT BOX IN DEBUG FOR TESTING - START
		//Drawing the debug box with the quat(Quaternion) for the rotation
		DrawDebugBox(GetWorld(), box->GetComponentLocation(), box->GetScaledBoxExtent(), box->GetComponentQuat(), FColor::Purple, false, 3.0f, 0, 4);
		//DRAWING THE CUTOUT BOX IN DEBUG FOR TESTING - END
		//
		TArray<AActor*> overLappingComponents;
		box->GetOverlappingActors(overLappingComponents);
		//START OF MESH SLICING
		for (AActor* comp : overLappingComponents)
		{

			UProceduralMeshComponent* procMeshComp = Cast<UProceduralMeshComponent>(comp->GetComponentByClass(UProceduralMeshComponent::StaticClass()));
			if (procMeshComp)
			{
				for (int i = 0; i < 4; i++)
				{
					FVector planePosition = box->GetComponentLocation();
					FVector planeNormal;
					FVector directionVector;

					switch (i)
					{
					case 0:
						UE_LOG(LogTemp, Warning, TEXT("SLICED LEFT"));

						// Moving the slicing plane left, With a normal pointing right
						directionVector = box->GetRightVector();
						directionVector *= FMath::Max(box->GetScaledBoxExtent().Y * 0.95f,10); //T0 Stop crashing 
						planePosition -= directionVector;

						planeNormal = box->GetRightVector();

						break;
					case 1:
						UE_LOG(LogTemp, Warning, TEXT("SLICED FORWARD"));

						// Moving the slicing plane forward, with a  normal pointing backward
						directionVector = box->GetForwardVector();
						directionVector *= FMath::Max(box->GetScaledBoxExtent().X *0.95f, 10); //T0 Stop crashing 
						planePosition += directionVector;

						planeNormal = -box->GetForwardVector();

						break;
					case 2:
						UE_LOG(LogTemp, Warning, TEXT("SLICED LEFT"));

						// Moving the slicing plane right, with a normal pointing left
						directionVector = box->GetRightVector();
						directionVector *= FMath::Max(box->GetScaledBoxExtent().Y * 0.95f, 10); //T0 Stop crashing 
						planePosition += directionVector;

						planeNormal = -box->GetRightVector();

						break;
					case 3:
						UE_LOG(LogTemp, Warning, TEXT("SLICED FORWARD"));

						// Moving the slicing plane backward, with a normal pointing forward
						directionVector =box->GetForwardVector();
						directionVector *= FMath::Max(box->GetScaledBoxExtent().X * 0.95f, 10); //T0 Stop crashing 
						planePosition -= directionVector;

						planeNormal = box->GetForwardVector();

						break;

					default:
						UE_LOG(LogTemp, Warning, TEXT("NOT SLICING ---ERROR---"));

					}
					UProceduralMeshComponent* otherHalfProcMesh = nullptr;
					UKismetProceduralMeshLibrary::SliceProceduralMesh(procMeshComp, planePosition, planeNormal, true, otherHalfProcMesh, EProcMeshSliceCapOption::CreateNewSectionForCap, box->GetMaterial(0));
					otherCutProcMeshes.Add(otherHalfProcMesh);					//Slicing The Mesh
				}
				//NEED TO ADD VARIABLES TO THE MESH
				procMeshComp->SetSimulatePhysics(true);
			}
		}
	}
	m_isCutting = false;

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
		DrawDebugLine(GetWorld(), start, hitResult.Location, FColor::Green, false, 0.1f, 0, 1.0f);
		// Check if something was hit
		if (hitResult.GetActor())
		{
			//UE_LOG(LogTemp, Warning, TEXT("Hit: %s"), *hitResult.GetActor()->GetName());
			m_lastImpactPoint = hitResult.ImpactPoint;
			//getting the procedural mesh component from the actor
			UProceduralMeshComponent* ProcMeshComp = Cast<UProceduralMeshComponent>(hitResult.GetActor()->GetComponentByClass(UProceduralMeshComponent::StaticClass()));
			if (ProcMeshComp)//making sure there is a procedural mesh component
			{
				if (!m_isCutting) {

					m_isCutting = true;
					m_boxOrigin = hitResult.ImpactPoint;


#pragma region Making the Rotation from Y to Z

					FVector normalizedY = this->GetActorRightVector().GetSafeNormal();

					FVector normalizedZ = hitResult.ImpactNormal.GetSafeNormal();

					// Calculate the X vector 
					FVector xVector = FVector::CrossProduct(normalizedY, normalizedZ).GetSafeNormal();
					//Calculate the Normalized Y using 
					normalizedY = FVector::CrossProduct(normalizedZ, xVector).GetSafeNormal();

					// Create a rotation from the orthonormal basis
					FMatrix rotationMatrix = FMatrix(xVector, normalizedY, normalizedZ, FVector::ZeroVector);
					FRotator rewRotation = rotationMatrix.Rotator();
#pragma endregion
					m_boxRotation = rewRotation;
					if (hitResult.GetActor()->FindComponentByClass<UProceduralMeshComponent>()) {
						m_hitActorCutable = true;
						UE_LOG(LogTemp, Warning, TEXT("HIT ACTOR CUTTABLE"));

					}
					else {
						m_hitActorCutable = false;
						UE_LOG(LogTemp, Warning, TEXT("HIT ACTOR NOT CUTTABLE"));


					}
				}
			}
		}
	}
}

void AUni_CuttingMeshes_Character::SetUpDebug()
{
	if (m_hitActorCutable) {
		box->SetWorldLocationAndRotation(m_boxOrigin, m_boxRotation);
		//Maketransform
		FTransform newTransform =  FTransform(m_boxRotation, m_boxOrigin, FVector(1,1,1));
		FTransform new2Transform = FTransform(m_boxRotation, m_lastImpactPoint, FVector(1, 1, 1));

		new2Transform.InverseTransformPosition(newTransform.GetLocation());
		FVector newLocation = FVector(FMath::Abs(new2Transform.GetRelativeTransform(newTransform).GetLocation().X), FMath::Abs(new2Transform.GetRelativeTransform(newTransform).GetLocation().Y), m_boxWidth);
		box->SetBoxExtent(newLocation, true);
		DrawDebugBox(GetWorld(),box->GetComponentLocation(),box->GetScaledBoxExtent(),box->GetComponentQuat(), FColor::Green, false, 0, 0, 10);
	}	
	//NEXT PART (CURRENTLY WORKING ON THIS)
}

void AUni_CuttingMeshes_Character::PickedUp(AActor* attachTo)
{
	this->AttachToActor(attachTo, FAttachmentTransformRules::KeepRelativeTransform);
	m_pickedUp = true;
}
