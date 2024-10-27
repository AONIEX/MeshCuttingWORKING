// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ProceduralMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "Uni_CuttingMeshes_Character.generated.h"

UCLASS()

class MESHCUTTING_API AUni_CuttingMeshes_Character : public ACharacter
{
	GENERATED_BODY()


	struct _MeshReturnInfo
	{

		UPROPERTY()
		bool bShouldReturn;

		UPROPERTY()
		FVector newLocation;
		UPROPERTY()
		FQuat newQuat;
		UPROPERTY()
		bool turnOnPhysics;

		UPROPERTY()
		bool finalReturn = false;
		// Constructor
		_MeshReturnInfo(): bShouldReturn(false), newLocation(FVector::ZeroVector), newQuat(FQuat::Identity) ,turnOnPhysics(false) {}
	};

public:
	// Sets default values for this character's properties
	AUni_CuttingMeshes_Character();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	//Variables
	bool m_returnAll = false;
	float m_boxWidth = 5;
	FVector m_lastImpactPoint;
	FVector m_boxOrigin;
	FRotator m_boxRotation;

	UPROPERTY(EditAnywhere, Category = "Mesh Cutting")
	TSubclassOf<AActor> m_procMeshRespawn;
	UPROPERTY(EditAnywhere, Category = "Mesh Cutting")
	bool canCut = true;
	UPROPERTY(EditAnywhere, Category = "Mesh Cutting")
	bool m_isCutting = false;
	UPROPERTY(EditAnywhere, Category = "Mesh Cutting")
	bool m_hitActorCutable = false;
	UPROPERTY(EditAnywhere, Category = "Mesh Cutting")
	bool m_holding = false;
	UPROPERTY(BlueprintReadOnly, Category = "Mesh Cutting")
	bool m_pickedUp = false;
	UPROPERTY(BlueprintReadOnly, Category = "Mesh Cutting")
	float cutOutDistance = 50;

	//Lists
	TArray<UProceduralMeshComponent*> m_cutMeshes;
	TMap<UProceduralMeshComponent*, _MeshReturnInfo> m_returningMeshes;


	//TArray<FVector>	m_cutMeshesOrigin;
	//TArray<FRotator> m_cutMeshesRoation;
	TArray<AActor> m_lastHitActor;

	//TArray<UProceduralMeshComponent*> otherCutProcMeshes;
	//open and closing gate
	bool m_gateOpen = false;
	UPROPERTY(EditAnywhere, Category = "Mesh Returining")
	float m_goToSpeed = 3;
	UPROPERTY(EditAnywhere, Category = "Mesh Cutting")
	UBoxComponent* m_box;
	UPROPERTY(EditAnywhere, Category = "Mesh Cutting")
	UStaticMeshComponent* m_boxMesh;
	UPROPERTY(EditAnywhere, Category = "Components")
	UActorComponent* m_hitComponent;
	UPROPERTY(EditAnywhere, Category = "Mesh Cutting")
	FString grabTag = "Grabbable";
	UPROPERTY(EditAnywhere, Category = "Mesh Cutting")
	FString cutTag = "Cut";
	UPROPERTY(EditAnywhere, Category = "Mesh Cutting")
	UPrimitiveComponent* m_grabbedComponent;
	UPROPERTY(EditAnywhere, Category = "Mesh Cutting")
	UPhysicsHandleComponent* m_physicsHandle;
	UPROPERTY(EditAnywhere, Category = "Mesh Cutting")
	USceneComponent* m_grabPoint;

	UPROPERTY(EditAnywhere, Category = "Mesh Cutting")
	USpringArmComponent* m_springArm;

	UPROPERTY(EditAnywhere, Category = "Mesh Cutting")
	UCameraComponent* m_cameraComponent;

	UPROPERTY(EditAnywhere, Category = "Mesh Cutting")
	float m_grabRange = 5000;
	float m_holdingOffSet = 100;


	bool m_debug = false;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void Start_Cutting();
	void Stop_Cutting();
	void SetUpCutting();
	void SetUpBoxAndDebug();
	void StartReturningAll();
	void ReturnAllToOriginalPosition(float dt);
	bool GoToPosition(TPair<UProceduralMeshComponent*, _MeshReturnInfo> returningCompMap,float dt, float speed);
	void Grab();
	void HoldObject();
	void StopGrabbing();


	//UFUNCTION(BlueprintCallable, category = "MyBlueprintLibary")
	//void PickedUp(AActor* attackToComponent);

	
};
