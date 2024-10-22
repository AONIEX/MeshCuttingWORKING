// Fill out your copyright notice in the Description page of Project Settings.


#include "Uni_ProcMesh.h"

// Sets default values
AUni_ProcMesh::AUni_ProcMesh()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    sceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
    RootComponent = sceneComponent;
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
    //RootComponent = StaticMeshComponent;
    StaticMeshComponent->SetupAttachment(RootComponent);

    RootComponent->SetVisibility(false);
    StaticMeshComponent->SetEnableGravity(true);
    StaticMeshComponent->SetSimulatePhysics(true);
    StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMeshComponent"));
    ProceduralMeshComponent->SetupAttachment(RootComponent);
   

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
    if (CubeMesh.Succeeded())
    {
        StaticMeshComponent->SetStaticMesh(CubeMesh.Object);
    }
   
}

// Called when the game starts or when spawned
void AUni_ProcMesh::BeginPlay()
{
	Super::BeginPlay();

	CopyMeshFromStaticMesh();
    RootComponent->SetVisibility(false);
    StaticMeshComponent->SetVisibility(false);
    StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    StaticMeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);

    UKismetProceduralMeshLibrary::CopyProceduralMeshFromStaticMeshComponent(StaticMeshComponent, 0, ProceduralMeshComponent, true);
    ProceduralMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    ProceduralMeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
    ProceduralMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
}

// Called every frame
void AUni_ProcMesh::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AUni_ProcMesh::CopyMeshFromStaticMesh()
{
 
}

