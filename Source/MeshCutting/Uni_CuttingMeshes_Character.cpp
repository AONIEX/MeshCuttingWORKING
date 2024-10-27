// Fill out your copyright notice in the Description page of Project Settings.


#include "Uni_CuttingMeshes_Character.h"




// Sets default values
AUni_CuttingMeshes_Character::AUni_CuttingMeshes_Character()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
// Create the Box Component
	m_box = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));

	//RootComponent = box;
	// Set the size of the box
	m_box->SetBoxExtent(FVector(32.0f, 32.0f, 32.0f));
	m_box->SetGenerateOverlapEvents(true);
	m_box->RegisterComponent();

	m_boxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	m_boxMesh->SetupAttachment(m_box);
	m_boxMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	if (MeshAsset.Succeeded())
	{
		m_boxMesh->SetStaticMesh(MeshAsset.Object);
	}

	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_springArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	m_springArm->SetupAttachment(RootComponent);
	m_springArm->TargetArmLength = 0;
	m_springArm->bUsePawnControlRotation = true; 

	// Create the Camera Component
	m_cameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	m_cameraComponent->SetupAttachment(m_springArm);
	m_cameraComponent->bUsePawnControlRotation = true;
	m_cameraComponent->SetRelativeLocation(FVector(0, 0, 55),false)
		;
	m_physicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
	m_physicsHandle->LinearDamping = 200.0f;
	m_physicsHandle->LinearStiffness = 750.0f;
	m_physicsHandle->AngularDamping = 500.0f;
	m_physicsHandle->AngularStiffness = 1500.0;
	m_physicsHandle->InterpolationSpeed = 50.0f;
	m_physicsHandle->bSoftAngularConstraint = true;
	m_physicsHandle->bSoftLinearConstraint = true;
	m_physicsHandle->bInterpolateTarget = true;
	m_physicsHandle->bAutoActivate = true;
	m_physicsHandle->RegisterComponent();

	m_grabPoint = CreateDefaultSubobject<USceneComponent>(TEXT("GrabPoint"));
	m_grabPoint->SetupAttachment(RootComponent); // Ensure it's attached to the root
	m_grabPoint->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f)); // Adjust the location if needed
	m_grabPoint->SetupAttachment(m_springArm);

}

// Called when the game starts or when spawned
void AUni_CuttingMeshes_Character::BeginPlay()
{
	Super::BeginPlay();
	m_box->SetCollisionProfileName(TEXT("OverlapAll"));
	m_isCutting = false;

}

// Called every frame
void AUni_CuttingMeshes_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (m_gateOpen) {

		SetUpCutting();
		SetUpBoxAndDebug();
		//Visualisng DebugBox And Cut Area
	}

	if (m_holding) {
		if (m_grabbedComponent) {

			HoldObject();
		}
	}
	else {
		if (!m_returnAll) {
			for (auto& Pair : m_returningMeshes)
			{
				if (Pair.Value.bShouldReturn)
				{
					Pair.Value.bShouldReturn= GoToPosition(Pair, DeltaTime, m_goToSpeed);

				}
			}
		}

		if (m_returnAll) {
			ReturnAllToOriginalPosition(DeltaTime);
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
	PlayerInputComponent->BindAction("ReturnToPosition", IE_Pressed, this, &AUni_CuttingMeshes_Character::StartReturningAll);
	PlayerInputComponent->BindAction("Grab", IE_Pressed, this, &AUni_CuttingMeshes_Character::Grab);
	PlayerInputComponent->BindAction("Grab", IE_Released, this, &AUni_CuttingMeshes_Character::StopGrabbing);


	//d}

}
#pragma region StartCutting
void AUni_CuttingMeshes_Character::Start_Cutting()
{
	if (canCut) {
		m_gateOpen = true;
		//if (pickedUp) {
		m_box->SetCollisionProfileName(TEXT("OverlapAll"));
		//}
		// --LOOK INTO--
		////Might also need to update OverLaps
	}
}
#pragma endregion
#pragma region StopCutting


void AUni_CuttingMeshes_Character::Stop_Cutting()
{
	if (canCut) {
		m_gateOpen = false;

		//if (pickedUp) {

		UE_LOG(LogTemp, Warning, TEXT("Stop_Cutting called"));
		m_hitActorCutable = false;

		if (m_isCutting) {
			//
			//DRAWING THE CUTOUT BOX IN DEBUG FOR TESTING - START
			//Drawing the debug box with the quat(Quaternion) for the rotation
			if (m_debug)
				DrawDebugBox(GetWorld(), m_box->GetComponentLocation(), m_box->GetScaledBoxExtent(), m_box->GetComponentQuat(), FColor::Purple, false, 3.0f, 0, 4);
			//DRAWING THE CUTOUT BOX IN DEBUG FOR TESTING - END
			//
			TArray<UPrimitiveComponent*> overLappingComponents;
			m_box->GetOverlappingComponents(overLappingComponents);
			//START OF MESH SLICING
			for (UPrimitiveComponent* comp : overLappingComponents)
			{
				UProceduralMeshComponent* procMeshComp = Cast<UProceduralMeshComponent>(comp);
				//UProceduralMeshComponent* procMeshComp = Cast<UProceduralMeshComponent>(comp->GetAttachmentRootActor()->GetComponentByClass(UProceduralMeshComponent::StaticClass()));

				if (procMeshComp)
				{
					for (int i = 0; i < 4; i++)
					{
						FVector planePosition = m_box->GetComponentLocation();
						FVector planeNormal;
						FVector directionVector;

						switch (i)
						{
						case 0:
							UE_LOG(LogTemp, Warning, TEXT("SLICED LEFT"));

							// Moving the slicing plane left, With a normal pointing right
							directionVector = m_box->GetRightVector();
							directionVector *= FMath::Max(m_box->GetScaledBoxExtent().Y, 10); //T0 Stop crashing 
							planePosition -= directionVector;

							planeNormal = m_box->GetRightVector();

							break;
						case 1:
							UE_LOG(LogTemp, Warning, TEXT("SLICED FORWARD"));

							// Moving the slicing plane forward, with a  normal pointing backward
							directionVector = m_box->GetForwardVector();
							directionVector *= FMath::Max(m_box->GetScaledBoxExtent().X, 10); //T0 Stop crashing 
							planePosition += directionVector;

							planeNormal = -m_box->GetForwardVector();

							break;
						case 2:
							UE_LOG(LogTemp, Warning, TEXT("SLICED LEFT"));

							// Moving the slicing plane right, with a normal pointing left
							directionVector = m_box->GetRightVector();
							directionVector *= FMath::Max(m_box->GetScaledBoxExtent().Y, 10); //T0 Stop crashing 
							planePosition += directionVector;

							planeNormal = -m_box->GetRightVector();

							break;
						case 3:
							UE_LOG(LogTemp, Warning, TEXT("SLICED FORWARD"));

							// Moving the slicing plane backward, with a normal pointing forward
							directionVector = m_box->GetForwardVector();
							directionVector *= FMath::Max(m_box->GetScaledBoxExtent().X, 10); //T0 Stop crashing 
							planePosition -= directionVector;

							planeNormal = m_box->GetForwardVector();

							break;

						default:
							UE_LOG(LogTemp, Warning, TEXT("NOT SLICING ---ERROR---"));

						}
						UProceduralMeshComponent* otherHalfProcMesh = nullptr;
					
						//otherCutProcMeshes.Add(otherHalfProcMesh);					//Slicing The Mesh
						UKismetProceduralMeshLibrary::SliceProceduralMesh(procMeshComp, planePosition, planeNormal, true, otherHalfProcMesh, EProcMeshSliceCapOption::CreateNewSectionForCap, m_box->GetMaterial(0));
						/*if (!m_cutMeshes.Contains(otherHalfProcMesh)) {
							m_cutMeshes.Add(otherHalfProcMesh);
						}*/

						if (!m_returningMeshes.Contains(otherHalfProcMesh)) {
							_MeshReturnInfo meshReturnInfo;
							meshReturnInfo.bShouldReturn = false;
							meshReturnInfo.newLocation = otherHalfProcMesh->GetComponentLocation();
							meshReturnInfo.newQuat = procMeshComp->GetComponentQuat();
							meshReturnInfo.turnOnPhysics = false;
							if (!m_cutMeshes.Contains(procMeshComp)) {
								m_cutMeshes.Add(procMeshComp);
							}
							m_returningMeshes.Add(otherHalfProcMesh, meshReturnInfo);

						}
						if (procMeshComp->ComponentTags.Contains(grabTag) || procMeshComp->ComponentTags.Contains(cutTag)) {
							otherHalfProcMesh->ComponentTags.Add(FName(cutTag));
						}

					}
					//NEED TO ADD VARIABLES TO THE MESH
					procMeshComp->SetSimulatePhysics(true);
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
							if (!component->ComponentTags.Contains(grabTag) && !component->ComponentTags.Contains("Cut")) {
								FBox componentBox = component->GetStreamingBounds();
								boundingBox += componentBox;
							}
						}
					}
					else {
						boundingBox = procMeshComp->GetStreamingBounds();
					}
					FVector upVector = m_box->GetUpVector();

					// Assuming we want to get the size projected in the direction of the up vector
					float projectedSize = FVector::DotProduct(boundingBox.GetSize(), upVector);
					m_box->SetBoxExtent(m_box->GetScaledBoxExtent() * 0.95f, true);
					m_boxMesh->SetRelativeScale3D(m_box->GetScaledBoxExtent() / 50.0f); //Dividing by 50 causes it to be the same size of the box extenet


					procMeshComp->ComponentTags.Add(FName(grabTag));
					procMeshComp->ComponentTags.Add(FName(cutTag));
			
					//Setting Up Mesh Returing and putting the mesh into a new location
					if (!m_returningMeshes.Contains(procMeshComp)) {

						_MeshReturnInfo meshReturnInfo;
						meshReturnInfo.bShouldReturn = true;
						meshReturnInfo.newLocation = procMeshComp->GetComponentLocation() + (m_box->GetUpVector() * ((upVector * projectedSize * 1.1f)));
						meshReturnInfo.newQuat = procMeshComp->GetComponentQuat();
						meshReturnInfo.turnOnPhysics = true;
						m_returningMeshes.Add(procMeshComp, meshReturnInfo);
					}
					else {
						m_returningMeshes.Find(procMeshComp)->bShouldReturn = true;
						m_returningMeshes.Find(procMeshComp)->newLocation = procMeshComp->GetComponentLocation() + (m_box->GetUpVector() * ((upVector * projectedSize * 1.1f)));
						m_returningMeshes.Find(procMeshComp)->newQuat = procMeshComp->GetComponentQuat();
						m_returningMeshes.Find(procMeshComp)->turnOnPhysics = true;
					}
					
				}
			}
		}
		m_isCutting = false;
		m_box->SetBoxExtent(FVector(0, 0, 0), true);
		m_boxMesh->SetRelativeScale3D(m_box->GetScaledBoxExtent()); 
		m_box->SetCollisionProfileName(TEXT("NoCollision"), true);
	}
	//}
}
#pragma endregion
#pragma region SetUps
void AUni_CuttingMeshes_Character::SetUpCutting()
{
	//Setting up Variables

	//ForTesting
	FVector start = GetActorLocation();
	//For Final Use 
	//FVector Start = cameraComponent->GetComponentLocation();
	FVector end = start + m_cameraComponent->GetForwardVector() * 5000;
	FHitResult hitResult;
	FCollisionQueryParams collisionParams;
	collisionParams.AddIgnoredActor(this);
	if (GetWorld()->LineTraceSingleByChannel(hitResult, start, end, ECC_Visibility, collisionParams))
	{
		if (m_debug)
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

void AUni_CuttingMeshes_Character::SetUpBoxAndDebug()
{
	if (m_hitActorCutable) {
		m_box->SetWorldLocationAndRotation(m_boxOrigin, m_boxRotation);
		//Maketransform
		FTransform newTransform =  FTransform(m_boxRotation, m_boxOrigin, FVector(1,1,1));
		FTransform new2Transform = FTransform(m_boxRotation, m_lastImpactPoint, FVector(1, 1, 1));

		new2Transform.InverseTransformPosition(newTransform.GetLocation());
		FVector newLocation = FVector(FMath::Abs(new2Transform.GetRelativeTransform(newTransform).GetLocation().X), FMath::Abs(new2Transform.GetRelativeTransform(newTransform).GetLocation().Y), m_boxWidth);
		m_box->SetBoxExtent(newLocation, true);
		FVector newBoxMeshScale = m_box->GetScaledBoxExtent() / 50.0f;
		m_boxMesh->SetRelativeScale3D(FVector(FMath::Max(newBoxMeshScale.X,0.2f),FMath::Max(newBoxMeshScale.Y,0.2f),newBoxMeshScale.Z)); //Dividing by 50 causes it to be the same size of the box extenet

		if(m_debug)
			DrawDebugBox(GetWorld(), m_box->GetComponentLocation(), m_box->GetScaledBoxExtent(), m_box->GetComponentQuat(), FColor::Green, false, 0, 0, 10);
	}	
	//NEXT PART (CURRENTLY WORKING ON THIS)
}

#pragma endregion
#pragma region Return_And_GoTo_Positions
void AUni_CuttingMeshes_Character::StartReturningAll()
{
	if (!m_isCutting) {

		for (auto& Pair : m_returningMeshes)
		{
			if (Pair.Value.bShouldReturn)
				Pair.Value.bShouldReturn = false;
		}
		m_returnAll = true;
		canCut = false;
	}
}

void AUni_CuttingMeshes_Character::ReturnAllToOriginalPosition(float dt)
{

	int returnsCompleted = 0;
	for (auto& Pair : m_returningMeshes) {

		UProceduralMeshComponent* procMesh = Pair.Key;
		FVector goToLocation = Pair.Value.newLocation;
		FQuat goToRotation = procMesh->GetAttachmentRootActor()->GetActorQuat();
		procMesh->SetSimulatePhysics(false);

		FVector currentLocation = procMesh->GetComponentLocation();
		const FQuat currentRotation = procMesh->GetComponentQuat();

		FVector newLocation = Pair.Key->GetComponentLocation();
		FQuat newRotation = Pair.Key->GetComponentQuat();
		
		const FQuat targetRotation = FQuat(goToRotation);
		float angularDistance = currentRotation.AngularDistance(targetRotation);

		// Log the angular distance for debugging

		// Define a threshold for rotation completion (in radians)
		const float rotationThreshold = FMath::DegreesToRadians(0.5f); // Adjust as need
			//if (angularDistance > rotationThreshold) {
			//}
			//else {
		if (Pair.Value.finalReturn) {
			// Optionally stop further movement
			procMesh->SetWorldLocation(goToLocation);
			procMesh->SetWorldRotation(goToRotation);
			FVector finalGoToLocation = procMesh->GetAttachmentRootActor()->GetActorLocation();
			FVector finalLocation = procMesh->GetComponentLocation();
			finalLocation = FMath::Lerp(currentLocation, finalGoToLocation, dt * m_goToSpeed);
			procMesh->SetWorldLocation(finalLocation);


			if (FVector::Dist(currentLocation, finalGoToLocation) < 1.0f) 
			{
				procMesh->SetWorldLocation(finalGoToLocation);
				returnsCompleted++;

			}
		}
		else {

			if (FMath::Abs(newLocation.Z - goToLocation.Z) > 0.5f )
			{
				newLocation.Z = FMath::Lerp(currentLocation.Z, goToLocation.Z, dt * m_goToSpeed);
			}
					else if (FMath::Abs(newLocation.X - goToLocation.X) > 0.25f)
					{
						newLocation.X = FMath::Lerp(currentLocation.X, goToLocation.X, dt * m_goToSpeed);
					}

					else if (FMath::Abs(newLocation.Y - goToLocation.Y) > 0.25f)
					{
						newLocation.Y = FMath::Lerp(currentLocation.Y, goToLocation.Y, dt * m_goToSpeed);
					}
			newRotation = FQuat::Slerp(currentRotation, FQuat(goToRotation), dt * m_goToSpeed/3);

				

			procMesh->SetWorldRotation(newRotation);
			procMesh->SetWorldLocation(newLocation);

		
			if (FVector::Dist(newLocation, goToLocation) < 1.0f) 
			{
				// Optionally stop further movement
				procMesh->SetWorldLocation(goToLocation);
				procMesh->SetWorldRotation(goToRotation);
				Pair.Value.finalReturn = true;
			}
		}

	}
	if (returnsCompleted == m_returningMeshes.Num()) {
		canCut = true;
		m_returnAll = false;
		TArray<AActor*> allOwners;
		for (auto& cutMesh : m_returningMeshes) {
			AActor* meshsActor = cutMesh.Key->GetAttachmentRootActor();
			if (!allOwners.Contains(meshsActor)) {
				allOwners.Add(meshsActor);
			}
		}
		for (auto& actor : allOwners)
		{
			FTransform actorTransform = actor->GetTransform();
			FVector procMeshScale = Cast<UProceduralMeshComponent>(actor->GetComponentByClass(UProceduralMeshComponent::StaticClass()))->GetComponentScale();

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AActor* newProcMeshActor;
			if (m_procMeshRespawn) {
				newProcMeshActor = GetWorld()->SpawnActor<AActor>(m_procMeshRespawn, actor->GetTransform().GetLocation(), actor->GetTransform().Rotator(), SpawnParams);
			}
			else {
				newProcMeshActor = nullptr;
			}
			if (newProcMeshActor)
			{
				UProceduralMeshComponent* procMeshComponent = Cast<UProceduralMeshComponent>(newProcMeshActor->GetComponentByClass(UProceduralMeshComponent::StaticClass()));
				procMeshComponent->SetWorldScale3D(procMeshScale);

				procMeshComponent->RegisterComponent();
			}
			actor->Destroy();

		}
		
		m_returningMeshes.Empty();
		m_cutMeshes.Empty();
	}

}

bool AUni_CuttingMeshes_Character::GoToPosition(TPair<UProceduralMeshComponent*, _MeshReturnInfo> returnPair, float dt, float speed)
{
	UProceduralMeshComponent* procMesh = returnPair.Key;
	FVector goToLocation = returnPair.Value.newLocation;
	FQuat goToRotation = returnPair.Value.newQuat;
	procMesh->SetSimulatePhysics(false);

	FVector currentLocation = procMesh->GetComponentLocation();
	FQuat currentRotation = procMesh->GetComponentQuat();

	//lerps the roation, and position between current one  and the go to one
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
		return false;

	}
	return true;

}

#pragma endregion
#pragma region Grabbing
void AUni_CuttingMeshes_Character::Grab()
{
	if (!m_holding && !m_returnAll) {
		UCameraComponent* cameraComponent = this->FindComponentByClass<UCameraComponent>();

		//Getting the start and end location of the raycast
		FVector start = m_cameraComponent->GetComponentLocation();
		FVector end = start + m_cameraComponent->GetForwardVector() * m_grabRange;
		FHitResult hitResult;
		FCollisionQueryParams collisionParams;
		collisionParams.AddIgnoredActor(this);
		if (GetWorld()->LineTraceSingleByChannel(hitResult, start, end, ECC_Visibility, collisionParams))
		{
			//For Testing
			if (m_debug)
				DrawDebugLine(GetWorld(), start, hitResult.Location, FColor::Blue, false, 0.1f, 0, 1.0f);
			
			if (hitResult.GetComponent())
			{
				UPrimitiveComponent* hitComp = hitResult.GetComponent();
				UProceduralMeshComponent* procMeshComp = Cast<UProceduralMeshComponent>(hitComp);
				//checks if the procMesh exists and is simulating physics
				if (procMeshComp && procMeshComp->IsSimulatingPhysics())
				{
					if (procMeshComp->ComponentTags.Contains(grabTag)) {
						//adding the object to the ocrrect variables as well as the physics handle
						m_grabbedComponent = procMeshComp;
						m_grabbedComponent->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
						m_holding = true;
						m_grabbedComponent->WakeRigidBody();
						m_physicsHandle->GrabComponentAtLocation(procMeshComp, NAME_None, procMeshComp->GetComponentLocation());
					}
				}
			}
		}
	}
}

void AUni_CuttingMeshes_Character::HoldObject()
{
	FVector center = FVector::ZeroVector;
	int32 vertexCount = 0;

	// Loop through all sections (assuming you want the first section here)
	UProceduralMeshComponent* procMeshComp = Cast<UProceduralMeshComponent>(m_grabbedComponent);
	// Loop through all the sections manually if you know the section count
	for (int32 sectionIndex = 0; sectionIndex < procMeshComp->GetNumSections(); sectionIndex++)
	{
		const FProcMeshSection* section = procMeshComp->GetProcMeshSection(sectionIndex);
		if (section)
		{
			// Get the vertices from the section
			const TArray<FProcMeshVertex>& vertices = section->ProcVertexBuffer;


			// Get the vertices from the section

			// Sum up the vertex positions
			for (const FProcMeshVertex& vertex : vertices)
			{
				center += vertex.Position;
			}
			vertexCount += vertices.Num();
		}
	}

	// Calculate the average to find the center
	if (vertexCount > 0)
	{
		center /= vertexCount;
	}

	// Get the local bounds of the grabbed component to determine its center
	FBoxSphereBounds bounds = m_grabbedComponent->GetStreamingBounds();
	FVector sliceCenter = bounds.GetBox().GetCenter(); // Center of the sliced mesh

	// Get the grab point location
	FVector grabPointLocation = m_grabPoint->GetComponentLocation();

	// Calculate the offset from the slice center to the grab point
	FVector offset = sliceCenter;// center; // This should just be the slice center for X and Y

	// Calculate the final position based on the grab point's location, keeping the Z from the grab point
	FVector localOffset = FVector((grabPointLocation.X - offset.X), (grabPointLocation.Y - offset.Y), grabPointLocation.Z - (offset.Z + center.Z));

	// Rotate the local offset based on the actor's rotation
	FVector rotatedOffset = FQuat(FRotator(0, this->GetActorRotation().Yaw, 0)) * localOffset;

	rotatedOffset.X = rotatedOffset.X / m_holdingOffSet;
	rotatedOffset.Y = rotatedOffset.Y / m_holdingOffSet;
	rotatedOffset.Z = localOffset.Z;

	// Calculate the final position
	FVector finalPos = grabPointLocation + (rotatedOffset);


	// Set the world rotation of the grabbed component
	m_grabbedComponent->SetWorldRotation(FRotator(0, this->GetActorRotation().Yaw, 0));

	// Update the physics handle's target location
	m_physicsHandle->SetTargetLocation(finalPos); // Set the target location to the grab point
}

void AUni_CuttingMeshes_Character::StopGrabbing() {
	if (m_grabbedComponent) {
		//Removes the component from the physics ahandle and stops its velocity
		m_physicsHandle->ReleaseComponent();
		m_grabbedComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		m_grabbedComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
		m_grabbedComponent->SetPhysicsAngularVelocityInDegrees(FVector(0,0,0), false, NAME_None);
		m_grabbedComponent = nullptr;
	}
	m_holding = false;


}

#pragma endregion

