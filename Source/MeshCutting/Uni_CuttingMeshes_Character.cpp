// Fill out your copyright notice in the Description page of Project Settings.


#include "Uni_CuttingMeshes_Character.h"

#pragma region Setting Up Variables


// Sets default values
AUni_CuttingMeshes_Character::AUni_CuttingMeshes_Character()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
// Create the Box Component
	m_cuttingBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));

	//RootComponent = box;
	// Set the size of the box
	m_cuttingBox->SetBoxExtent(FVector(32.0f, 32.0f, 32.0f));
	m_cuttingBox->SetGenerateOverlapEvents(true);
	m_cuttingBox->RegisterComponent();

	m_boxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	m_boxMesh->SetupAttachment(m_cuttingBox);
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

	// Create and set up the Camera Component
	m_cameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	m_cameraComponent->SetupAttachment(m_springArm);
	m_cameraComponent->bUsePawnControlRotation = true;
	m_cameraComponent->SetRelativeLocation(FVector(0, 0, 55),false);

	// Create and set up the physics handle Component
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


	// Create and set up the grab point handle Component
	m_grabPoint = CreateDefaultSubobject<USceneComponent>(TEXT("GrabPoint"));
	m_grabPoint->SetupAttachment(RootComponent); 
	m_grabPoint->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f)); 
	m_grabPoint->SetupAttachment(m_springArm);

}
#pragma endregion
#pragma region Begin, Tick and Update


// Called when the game starts or when spawned
void AUni_CuttingMeshes_Character::BeginPlay()
{
	Super::BeginPlay();
	m_cuttingBox->SetCollisionProfileName(TEXT("OverlapAll"));
	m_isCutting = false;

}

// Called every frame
void AUni_CuttingMeshes_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (m_cuttingGateOpen) {//sets up the cutting and the box

		SetUpCutting();
		SetUpBoxAndDebug();
	}

	if (m_holding) {//calls the holding function if holding
		if (m_grabbedComponent) {

			HoldObject();
		}
	}
	else {
		if (!m_returnAll) {
			for (auto& Pair : m_returningMeshes)
			{
				if (Pair.Value.goToPosition)//makes cut meshes go to their go to locations if wanted
				{
					Pair.Value.goToPosition= GoToPosition(Pair, DeltaTime, m_goToSpeed);

				}
			}
		}

		if (m_returnAll) {// returns all cut meshes to their original positions
			ReturnAllToOriginalPosition(DeltaTime);
		}
	}
	
}

// Called to bind functionality to input
void AUni_CuttingMeshes_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	//Setting up all the inputs
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("StartCutting", IE_Pressed, this, &AUni_CuttingMeshes_Character::Start_Cutting);
	PlayerInputComponent->BindAction("StartCutting", IE_Released, this, &AUni_CuttingMeshes_Character::Stop_Cutting);
	PlayerInputComponent->BindAction("ReturnToPosition", IE_Pressed, this, &AUni_CuttingMeshes_Character::StartReturningAll);
	PlayerInputComponent->BindAction("Grab", IE_Pressed, this, &AUni_CuttingMeshes_Character::Grab);
	PlayerInputComponent->BindAction("Grab", IE_Released, this, &AUni_CuttingMeshes_Character::StopGrabbing);
}
#pragma endregion
#pragma region StartCutting
void AUni_CuttingMeshes_Character::Start_Cutting()
{
	if (canCut) {
		m_cuttingGateOpen = true;
		//if (pickedUp) {
		m_cuttingBox->SetCollisionProfileName(TEXT("OverlapAll"));
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
		m_cuttingGateOpen = false;

		UE_LOG(LogTemp, Warning, TEXT("Stop_Cutting called"));
		m_hitActorCutable = false;

		if (m_isCutting) {
			if (m_debug)//Drawing the box For Testing
				DrawDebugBox(GetWorld(), m_cuttingBox->GetComponentLocation(), m_cuttingBox->GetScaledBoxExtent(), m_cuttingBox->GetComponentQuat(), FColor::Purple, false, 3.0f, 0, 4);
			TArray<UPrimitiveComponent*> overLappingComponents;
			m_cuttingBox->GetOverlappingComponents(overLappingComponents);

			//cuts all of the procedural mesh's that the box overlapped with
			for (UPrimitiveComponent* comp : overLappingComponents)
			{
				UProceduralMeshComponent* procMeshComp = Cast<UProceduralMeshComponent>(comp);

				if (procMeshComp)
				{
					for (int i = 0; i < 4; i++)
					{
						FVector planePosition = m_cuttingBox->GetComponentLocation();
						FVector planeNormal;
						FVector directionVector;

						switch (i)
						{
						case 0:
							UE_LOG(LogTemp, Warning, TEXT("SLICED LEFT"));

							// moving the slicing plane left, With a normal pointing right
							directionVector = m_cuttingBox->GetRightVector();
							directionVector *= FMath::Max(m_cuttingBox->GetScaledBoxExtent().Y, 10); //T0 Stop crashing 
							planePosition -= directionVector;

							planeNormal = m_cuttingBox->GetRightVector();

							break;
						case 1:
							UE_LOG(LogTemp, Warning, TEXT("SLICED FORWARD"));

							// moving the slicing plane forward, with a  normal pointing backward
							directionVector = m_cuttingBox->GetForwardVector();
							directionVector *= FMath::Max(m_cuttingBox->GetScaledBoxExtent().X, 10); //T0 Stop crashing 
							planePosition += directionVector;

							planeNormal = -m_cuttingBox->GetForwardVector();

							break;
						case 2:
							UE_LOG(LogTemp, Warning, TEXT("SLICED LEFT"));

							// moving the slicing plane right, with a normal pointing left
							directionVector = m_cuttingBox->GetRightVector();
							directionVector *= FMath::Max(m_cuttingBox->GetScaledBoxExtent().Y, 10); //T0 Stop crashing 
							planePosition += directionVector;

							planeNormal = -m_cuttingBox->GetRightVector();

							break;
						case 3:
							UE_LOG(LogTemp, Warning, TEXT("SLICED FORWARD"));

							// moving the slicing plane backward, with a normal pointing forward
							directionVector = m_cuttingBox->GetForwardVector();
							directionVector *= FMath::Max(m_cuttingBox->GetScaledBoxExtent().X, 10); //T0 Stop crashing 
							planePosition -= directionVector;

							planeNormal = m_cuttingBox->GetForwardVector();

							break;

						default:
							UE_LOG(LogTemp, Warning, TEXT("NOT SLICING ---ERROR---"));

						}
						UProceduralMeshComponent* otherHalfProcMesh = nullptr;
					
						UKismetProceduralMeshLibrary::SliceProceduralMesh(procMeshComp, planePosition, planeNormal, true, otherHalfProcMesh, EProcMeshSliceCapOption::CreateNewSectionForCap, m_cuttingBox->GetMaterial(0));

						if (!m_returningMeshes.Contains(otherHalfProcMesh)) {
							//adds the  mesh to the returning meshes without telling it to move position
							_MeshReturnInfo meshReturnInfo;
							meshReturnInfo.goToPosition = false;
							meshReturnInfo.newLocation = otherHalfProcMesh->GetComponentLocation();
							meshReturnInfo.newQuat = procMeshComp->GetComponentQuat();
							meshReturnInfo.turnOnPhysics = false;
							if (!m_cutMeshes.Contains(procMeshComp)) {
								m_cutMeshes.Add(procMeshComp);
							}
							m_returningMeshes.Add(otherHalfProcMesh, meshReturnInfo);

						}
						if (procMeshComp->ComponentTags.Contains(grabTag) || procMeshComp->ComponentTags.Contains(cutTag)) {
							//adds the cut tag to the component if th mesh it waas cut from was cut
							otherHalfProcMesh->ComponentTags.Add(FName(cutTag));
						}

					}
					
					procMeshComp->SetSimulatePhysics(true);
					FBox boundingBox(ForceInit); 

					//going through all the procedural meshes to get the bounding box
					if (!procMeshComp->ComponentTags.Contains(cutTag)) {
						TArray<UProceduralMeshComponent*> allComponents;

				
						procMeshComp->GetAttachmentRootActor()->GetComponents(allComponents);

						for (UProceduralMeshComponent* component : allComponents)
						{
							//if the object cant be grabbed and hasnt been cut add it to the bounding box
							if (!component->ComponentTags.Contains(grabTag) && !component->ComponentTags.Contains("Cut")) {
								FBox componentBox = component->GetStreamingBounds();
								boundingBox += componentBox;
							}
						}
					}
					else {
						boundingBox = procMeshComp->GetStreamingBounds();
					}
					FVector upVector = m_cuttingBox->GetUpVector();

					// getting the size projected in the direction of the up vector
					float projectedSize = FVector::DotProduct(boundingBox.GetSize(), upVector);
					m_cuttingBox->SetBoxExtent(m_cuttingBox->GetScaledBoxExtent() * 0.95f, true);
					m_boxMesh->SetRelativeScale3D(m_cuttingBox->GetScaledBoxExtent() / 50.0f); //Dividing by 50 causes it to be the same size of the box extenet


					procMeshComp->ComponentTags.Add(FName(grabTag));
					procMeshComp->ComponentTags.Add(FName(cutTag));
			
					//setting Up Mesh Returing and putting the mesh into a new location
					if (!m_returningMeshes.Contains(procMeshComp)) {
						//adding to the returning meshes and setting up the variables
						_MeshReturnInfo meshReturnInfo;
						meshReturnInfo.goToPosition = true;
						meshReturnInfo.newLocation = procMeshComp->GetComponentLocation() + (m_cuttingBox->GetUpVector() * ((upVector * projectedSize * 1.1f)));
						meshReturnInfo.newQuat = procMeshComp->GetComponentQuat();
						meshReturnInfo.turnOnPhysics = true;
						m_returningMeshes.Add(procMeshComp, meshReturnInfo);
					}
					else {
						//setting up the variables to move this to a new position
						m_returningMeshes.Find(procMeshComp)->goToPosition = true;
						m_returningMeshes.Find(procMeshComp)->newLocation = procMeshComp->GetComponentLocation() + (m_cuttingBox->GetUpVector() * ((upVector * projectedSize * 1.1f)));
						m_returningMeshes.Find(procMeshComp)->newQuat = procMeshComp->GetComponentQuat();
						m_returningMeshes.Find(procMeshComp)->turnOnPhysics = true;
					}
					
				}
			}
		}
		m_isCutting = false;
		m_cuttingBox->SetBoxExtent(FVector(0, 0, 0), true);
		m_boxMesh->SetRelativeScale3D(m_cuttingBox->GetScaledBoxExtent()); 
		m_cuttingBox->SetCollisionProfileName(TEXT("NoCollision"), true);
	}
}
#pragma endregion
#pragma region Cutting SetUps
void AUni_CuttingMeshes_Character::SetUpCutting()
{

	FVector start = m_cameraComponent->GetComponentLocation();
	FVector end = start + m_cameraComponent->GetForwardVector() * m_cutAndGrabRange;
	FHitResult hitResult;
	FCollisionQueryParams collisionParams;
	collisionParams.AddIgnoredActor(this);

	//Using ray Trace to check if we have hit a object
	if (GetWorld()->LineTraceSingleByChannel(hitResult, start, end, ECC_Visibility, collisionParams))
	{
		
		if (m_debug)//For Testing
			DrawDebugLine(GetWorld(), start, hitResult.Location, FColor::Green, false, 0.1f, 0, 1.0f);

		if (hitResult.GetActor())
		{
			//Checking if that object has a procedural mesh component, which means its cuttable
			UProceduralMeshComponent* ProcMeshComp = Cast<UProceduralMeshComponent>(hitResult.GetActor()->GetComponentByClass(UProceduralMeshComponent::StaticClass()));
			if (ProcMeshComp)//making sure there is a procedural mesh component
			{
				m_lastImpactPoint = hitResult.ImpactPoint;

				if (!m_isCutting) {

					m_isCutting = true;
					m_boxOrigin = hitResult.ImpactPoint;


#pragma region Making the Rotation from Y to Z

					//Figuring out the rotation for the box based on where it has hit the other object
					FVector normalizedY = this->GetActorRightVector().GetSafeNormal();
					FVector normalizedZ = hitResult.ImpactNormal.GetSafeNormal();

					FVector xVector = FVector::CrossProduct(normalizedY, normalizedZ).GetSafeNormal();
					normalizedY = FVector::CrossProduct(normalizedZ, xVector).GetSafeNormal();
					
					// Create a rotation from the Normalized Y and Z
					FMatrix rotationMatrix = FMatrix(xVector, normalizedY, normalizedZ, FVector::ZeroVector);
					FRotator rewRotation = rotationMatrix.Rotator();
#pragma endregion
					m_boxRotation = rewRotation;
					m_hitActorCutable = true;
				}
			}
		}
	}
}

void AUni_CuttingMeshes_Character::SetUpBoxAndDebug()
{
	//Setting up the box based on th ebox origin and rotation
	if (m_hitActorCutable) {
		m_cuttingBox->SetWorldLocationAndRotation(m_boxOrigin, m_boxRotation);
		//Maketransform
		FTransform newTransform =  FTransform(m_boxRotation, m_boxOrigin, FVector(1,1,1));
		FTransform new2Transform = FTransform(m_boxRotation, m_lastImpactPoint, FVector(1, 1, 1));

		new2Transform.InverseTransformPosition(newTransform.GetLocation());
		//Getting the new location based off its current location and the last impact point
		FVector newLocation = FVector(FMath::Abs(new2Transform.GetRelativeTransform(newTransform).GetLocation().X), FMath::Abs(new2Transform.GetRelativeTransform(newTransform).GetLocation().Y), m_boxWidth);
		m_cuttingBox->SetBoxExtent(newLocation, true);

		FVector newBoxMeshScale = m_cuttingBox->GetScaledBoxExtent() / 50.0f;//Dividing by 50 causes it to be the same size of the box extenet
		m_boxMesh->SetRelativeScale3D(FVector(FMath::Max(newBoxMeshScale.X,0.2f),FMath::Max(newBoxMeshScale.Y,0.2f),newBoxMeshScale.Z)); 

		if(m_debug)
			DrawDebugBox(GetWorld(), m_cuttingBox->GetComponentLocation(), m_cuttingBox->GetScaledBoxExtent(), m_cuttingBox->GetComponentQuat(), FColor::Green, false, 0, 0, 10);
	}	
}

#pragma endregion
#pragma region Return_And_GoTo_Positions
void AUni_CuttingMeshes_Character::StartReturningAll()
{
	if (!m_isCutting) {
		//Makes sure the actors arent lerping to a new position before initialising the return all
		for (auto& Pair : m_returningMeshes)
		{
			if (Pair.Value.goToPosition)
				Pair.Value.goToPosition = false;
		}
		m_returnAll = true;
		canCut = false;
	}
}

void AUni_CuttingMeshes_Character::ReturnAllToOriginalPosition(float dt)
{

	int returnsCompleted = 0;
	for (auto& Pair : m_returningMeshes) {

		//Gettting all teh variables needed to return all cut meshes to their original osition
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


		//defining threshold for rotation
		const float rotationThreshold = FMath::DegreesToRadians(0.5f); 
		if (Pair.Value.returnToOriginalPos) {

			//procMesh->SetWorldLocation(goToLocation);
			//procMesh->SetWorldRotation(goToRotation);
			FVector finalGoToLocation = procMesh->GetAttachmentRootActor()->GetActorLocation();
			FVector finalLocation = procMesh->GetComponentLocation();

			//Lerping to the final location for the cut mesh
			finalLocation = FMath::Lerp(currentLocation, finalGoToLocation, dt * m_goToSpeed);
			procMesh->SetWorldLocation(finalLocation);


			if (FVector::Dist(currentLocation, finalGoToLocation) < 1.0f) //Checking if the cut mesh is in the final location
			{
				procMesh->SetWorldLocation(finalGoToLocation);
				returnsCompleted++;

			}
		}
		else {

			if (FMath::Abs(newLocation.Z - goToLocation.Z) > 0.5f )//Lerping the z  to the go to z
			{
				newLocation.Z = FMath::Lerp(currentLocation.Z, goToLocation.Z, dt * m_goToSpeed);
			}
					else if (FMath::Abs(newLocation.X - goToLocation.X) > 0.25f)//Lerping the X  to the go to x
					{
						newLocation.X = FMath::Lerp(currentLocation.X, goToLocation.X, dt * m_goToSpeed);
					}

					else if (FMath::Abs(newLocation.Y - goToLocation.Y) > 0.25f)//Lerping the Y  to the go to Y
					{
						newLocation.Y = FMath::Lerp(currentLocation.Y, goToLocation.Y, dt * m_goToSpeed);
					}
			newRotation = FQuat::Slerp(currentRotation, FQuat(goToRotation), dt * m_goToSpeed/3);//Lerping the rotation at the same time as the other lerping but at a thrid of the speed

			procMesh->SetWorldRotation(newRotation);
			procMesh->SetWorldLocation(newLocation);

		
			if (FVector::Dist(newLocation, goToLocation) < 1.0f) //Checking if the cut mesh is in the go to location
			{
				// Optionally stop further movement
				procMesh->SetWorldLocation(goToLocation);
				procMesh->SetWorldRotation(goToRotation);
				Pair.Value.returnToOriginalPos = true;
			}
		}

	}
	if (returnsCompleted == m_returningMeshes.Num()) {//If all the cut meshes are in there original location reset the mesh
		canCut = true;
		m_returnAll = false;
		TArray<AActor*> allOwners;
		for (auto& cutMesh : m_returningMeshes) {//getting all the actors of the cut meshes
			AActor* meshsActor = cutMesh.Key->GetAttachmentRootActor();
			if (!allOwners.Contains(meshsActor)) {
				allOwners.Add(meshsActor);
			}
		}
		for (auto& actor : allOwners)//Destroying the actor (and all cut meshes by doing so) and spawing in a new replica wihtout any cuts
		{
			//Getting all the variables for the new mesh to be spawned
			FTransform actorTransform = actor->GetTransform();
			FVector procMeshScale = Cast<UProceduralMeshComponent>(actor->GetComponentByClass(UProceduralMeshComponent::StaticClass()))->GetComponentScale();
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AActor* newProcMeshActor;
			if (m_procMeshRespawn) {//Creating the new cuttable actor
				newProcMeshActor = GetWorld()->SpawnActor<AActor>(m_procMeshRespawn, actor->GetTransform().GetLocation(), actor->GetTransform().Rotator(), SpawnParams);
			}
			else {
				newProcMeshActor = nullptr;
			}
			if (newProcMeshActor)//Scaling the new cuttable actor
			{
				UProceduralMeshComponent* procMeshComponent = Cast<UProceduralMeshComponent>(newProcMeshActor->GetComponentByClass(UProceduralMeshComponent::StaticClass()));
				procMeshComponent->SetWorldScale3D(procMeshScale);

				procMeshComponent->RegisterComponent();
			}
			actor->Destroy();//Destroying the old cuttable actor

		}
		
		m_returningMeshes.Empty();
		m_cutMeshes.Empty();
	}

}

bool AUni_CuttingMeshes_Character::GoToPosition(TPair<UProceduralMeshComponent*, _MeshReturnInfo> returnPair, float dt, float speed)
{
	//getting all the variables for going to a new position
	UProceduralMeshComponent* procMesh = returnPair.Key;
	FVector goToLocation = returnPair.Value.newLocation;
	FQuat goToRotation = returnPair.Value.newQuat;
	procMesh->SetSimulatePhysics(false);

	FVector currentLocation = procMesh->GetComponentLocation();
	FQuat currentRotation = procMesh->GetComponentQuat();

	//lerps the roation, and position between current position  and the go to position
	FVector newLocation = FMath::Lerp(currentLocation, goToLocation, dt * speed);
	FQuat newRotation = FQuat::Slerp(currentRotation, FQuat(goToRotation), dt * speed);

	procMesh->SetWorldRotation(newRotation);
	procMesh->SetWorldLocation(newLocation);

	if (FVector::Dist(newLocation, goToLocation) < 0.5f) //checks to make sure the cut mesh is in the new position
	{
		procMesh->SetWorldLocation(goToLocation);
		procMesh->SetWorldRotation(goToRotation);
		if (returnPair.Value.turnOnPhysics == true) {//turing on physics if needed
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
		FVector end = start + m_cameraComponent->GetForwardVector() * m_cutAndGrabRange;
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

	UProceduralMeshComponent* procMeshComp = Cast<UProceduralMeshComponent>(m_grabbedComponent);
	// getting all the vertices to figure out the centre of the procedurally cut mesh
	for (int32 sectionIndex = 0; sectionIndex < procMeshComp->GetNumSections(); sectionIndex++)
	{
		const FProcMeshSection* section = procMeshComp->GetProcMeshSection(sectionIndex);
		if (section)
		{
			const TArray<FProcMeshVertex>& vertices = section->ProcVertexBuffer;

			// adding all the vertex positions
			for (const FProcMeshVertex& vertex : vertices)
			{
				center += vertex.Position;
			}
			vertexCount += vertices.Num();
		}
	}

	// getting the average in order to get the centre
	if (vertexCount > 0)
	{
		center /= vertexCount;
	}

	//setting up variables for grabbing at the correct position
	FBoxSphereBounds bounds = m_grabbedComponent->GetStreamingBounds();
	FVector grabPointLocation = m_grabPoint->GetComponentLocation();
	FVector offset = bounds.GetBox().GetCenter();

	// getting the final position based on the grab point's location and adding the offset as well as the centre of the vertices
	FVector localOffset = FVector((grabPointLocation.X - offset.X), (grabPointLocation.Y - offset.Y), grabPointLocation.Z - (offset.Z + center.Z));

	FVector rotatedOffset = FQuat(FRotator(0, this->GetActorRotation().Yaw, 0)) * localOffset;

	rotatedOffset.X = rotatedOffset.X / m_holdingOffSet;
	rotatedOffset.Y = rotatedOffset.Y / m_holdingOffSet;
	rotatedOffset.Z = localOffset.Z;

	//getting the final position beased on the actors rotation
	FVector finalPos = grabPointLocation + (rotatedOffset);
	m_grabbedComponent->SetWorldRotation(FRotator(0, this->GetActorRotation().Yaw, 0));
	m_physicsHandle->SetTargetLocation(finalPos); 
}

void AUni_CuttingMeshes_Character::StopGrabbing() {
	//removes the component from the physics handle and stops its velocity
	if (m_grabbedComponent) {
		m_physicsHandle->ReleaseComponent();
		m_grabbedComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		m_grabbedComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
		m_grabbedComponent->SetPhysicsAngularVelocityInDegrees(FVector(0,0,0), false, NAME_None);
		m_grabbedComponent = nullptr;
	}
	m_holding = false;
}

#pragma endregion

