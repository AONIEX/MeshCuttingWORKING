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
	m_isCutting = false;

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


	for (auto& Pair : _returningMeshes)
	{
		if (Pair.Value.bShouldReturn)
		{
			GoToPosition(Pair, Pair.Value.bShouldReturn,DeltaTime, goToSpeed);

		}
	}
	

}

// Called to bind functionality to input
void AUni_CuttingMeshes_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	//if (pickedUp) {

	PlayerInputComponent->BindAction("StartCutting", IE_Pressed, this, &AUni_CuttingMeshes_Character::Start_Cutting);
	PlayerInputComponent->BindAction("StartCutting", IE_Released, this, &AUni_CuttingMeshes_Character::Stop_Cutting);
	PlayerInputComponent->BindAction("ReturnToPosition", IE_Released, this, &AUni_CuttingMeshes_Character::ReturnAllToOriginalPosition);

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
		TArray<UPrimitiveComponent*> overLappingComponents;
		box->GetOverlappingComponents(overLappingComponents);
		//START OF MESH SLICING
		for (UPrimitiveComponent* comp : overLappingComponents)
		{
			UProceduralMeshComponent* procMeshComp = Cast<UProceduralMeshComponent>(comp);
			//UProceduralMeshComponent* procMeshComp = Cast<UProceduralMeshComponent>(comp->GetAttachmentRootActor()->GetComponentByClass(UProceduralMeshComponent::StaticClass()));

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
						directionVector *= FMath::Max(box->GetScaledBoxExtent().Y ,10); //T0 Stop crashing 
						planePosition -= directionVector;

						planeNormal = box->GetRightVector();

						break;
					case 1:
						UE_LOG(LogTemp, Warning, TEXT("SLICED FORWARD"));

						// Moving the slicing plane forward, with a  normal pointing backward
						directionVector = box->GetForwardVector();
						directionVector *= FMath::Max(box->GetScaledBoxExtent().X , 10); //T0 Stop crashing 
						planePosition += directionVector;

						planeNormal = -box->GetForwardVector();

						break;
					case 2:
						UE_LOG(LogTemp, Warning, TEXT("SLICED LEFT"));

						// Moving the slicing plane right, with a normal pointing left
						directionVector = box->GetRightVector();
						directionVector *= FMath::Max(box->GetScaledBoxExtent().Y , 10); //T0 Stop crashing 
						planePosition += directionVector;

						planeNormal = -box->GetRightVector();

						break;
					case 3:
						UE_LOG(LogTemp, Warning, TEXT("SLICED FORWARD"));

						// Moving the slicing plane backward, with a normal pointing forward
						directionVector =box->GetForwardVector();
						directionVector *= FMath::Max(box->GetScaledBoxExtent().X , 10); //T0 Stop crashing 
						planePosition -= directionVector;

						planeNormal = box->GetForwardVector();

						break;

					default:
						UE_LOG(LogTemp, Warning, TEXT("NOT SLICING ---ERROR---"));

					}
					UProceduralMeshComponent* otherHalfProcMesh = nullptr;
					
					//otherCutProcMeshes.Add(otherHalfProcMesh);					//Slicing The Mesh
					UKismetProceduralMeshLibrary::SliceProceduralMesh(procMeshComp, planePosition, planeNormal, true, otherHalfProcMesh, EProcMeshSliceCapOption::CreateNewSectionForCap, box->GetMaterial(0));
					m_cutMeshes.Add(otherHalfProcMesh);
					if (procMeshComp->ComponentTags.Contains(tag) || procMeshComp->ComponentTags.Contains(cutTag)) {
						otherHalfProcMesh->ComponentTags.Add(FName(cutTag));
					}

				}
				//NEED TO ADD VARIABLES TO THE MESH
				procMeshComp->SetSimulatePhysics(true);
				//procMeshComp->SetWorldLocation(); //MAKING THIS INTO A LERP LATER ON TO MAKE A SMOOTH TRANSITION - TODO
				//m_cutMeshes.Add(procMeshComp);
				//FBoxSphereBounds boundingBox = procMeshComp->GetAttachmentRootActor()->GetComponentsBoundingBox();

				//// Step 2: Calculate the size
				//FVector size = boundingBox.GetBox().GetSize(); // size.x, size.y, size.z

				//// Step 3: Get the up vector of the reference actor
				//FVector upVector = box->GetUpVector();

				//// Step 4: Project the size onto the up vector
				//// Assuming we want to get the size projected in the direction of the up vector
				//float projectedSize = FVector::DotProduct(size, upVector);

				FBox boundingBox(ForceInit); // Initialize an empty bounding box
				if (!procMeshComp->ComponentTags.Contains(cutTag)) {
					TArray<UProceduralMeshComponent*> allComponents;

					// Get all components of the actor
						procMeshComp->GetAttachmentRootActor()->GetComponents(allComponents);

						// Create a set of components to exclude for faster lookup

						// Iterate through all components
						for (UProceduralMeshComponent* component : allComponents)
						{

							// Check if this component should be excluded
							// Expand the bounding box to include this component
							if (!component->ComponentTags.Contains(tag) && !component->ComponentTags.Contains("Cut")) {
								FBox componentBox = component->GetStreamingBounds();
								boundingBox += componentBox;
							}
						}
				}
				else {
					boundingBox = procMeshComp->GetStreamingBounds();
				}
				FVector upVector = box->GetUpVector();

				// Step 4: Project the size onto the up vector
				// Assuming we want to get the size projected in the direction of the up vector
				float projectedSize = FVector::DotProduct(boundingBox.GetSize(), upVector);
				box->SetBoxExtent(box->GetScaledBoxExtent() * 0.95f, true);
				procMeshComp->ComponentTags.Add(FName(tag));
				procMeshComp->ComponentTags.Add(FName(cutTag));


				//Setting Up Mesh Returing and putting the mesh into a new location
				_MeshReturnInfo meshReturnInfo;
				meshReturnInfo.bShouldReturn = true;
				meshReturnInfo.newLocation = procMeshComp->GetComponentLocation() + (box->GetUpVector() * ((upVector *projectedSize * 1.1f )));
				meshReturnInfo.newQuat = procMeshComp->GetComponentQuat();
				meshReturnInfo.turnOnPhysics = true;
				_returningMeshes.Add(procMeshComp, meshReturnInfo);
			}
		}
	}
	m_isCutting = false;
	box->SetBoxExtent(FVector(0,0,0), true);
	box->SetCollisionProfileName(TEXT("NoCollision"), true);

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

void AUni_CuttingMeshes_Character::ReturnAllToOriginalPosition()
{
	//returnToPos = true;
	/*for (int i = 0; i < m_cutMeshes.Num(); i++)
	{
		FVector originalLocation = m_cutMeshes[i]->GetAttachmentRootActor()->GetActorLocation();
		FRotator originalRotation = m_cutMeshes[i]->GetAttachmentRootActor()->GetActorRotation();
		m_cutMeshes[i]->SetSimulatePhysics(false);
		m_cutMeshes[i]->SetWorldRotation(originalRotation);
		m_cutMeshes[i]->SetWorldLocation(originalLocation);
	}*/

	//NEXT PART TO DO
}

void AUni_CuttingMeshes_Character::GoToPosition(TPair<UProceduralMeshComponent*, _MeshReturnInfo> returnPair, bool &shouldReturn, float dt, float speed)
{
	UProceduralMeshComponent* procMesh = returnPair.Key;
	FVector goToLocation = returnPair.Value.newLocation;
	FQuat goToRotation = returnPair.Value.newQuat;
	procMesh->SetSimulatePhysics(false);

	FVector currentLocation = procMesh->GetComponentLocation();
	FQuat currentRotation = procMesh->GetComponentQuat();

	FVector newLocation = FMath::Lerp(currentLocation, goToLocation, dt * speed);
	FQuat newRotation = FQuat::Slerp(currentRotation, FQuat(goToRotation), dt * speed);

	procMesh->SetWorldRotation(newRotation);
	procMesh->SetWorldLocation(newLocation);

	if (FVector::Dist(newLocation, goToLocation) < 0.5f) // Adjust the threshold as needed
	{
		// Optionally stop further movement
		procMesh->SetWorldLocation(goToLocation);
		procMesh->SetWorldRotation(goToRotation);
		if (returnPair.Value.turnOnPhysics == true) {
			procMesh->SetSimulatePhysics(true);
		}
		shouldReturn = false;
	}
}

//void AUni_CuttingMeshes_Character::PickedUp(AActor* attachTo)
//{
//	this->AttachToActor(attachTo, FAttachmentTransformRules::KeepRelativeTransform);
//	m_pickedUp = true;
//}
