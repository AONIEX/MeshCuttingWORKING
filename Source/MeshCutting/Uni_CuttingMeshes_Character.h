// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "Camera/CameraComponent.h"
#include "Uni_CuttingMeshes_Character.generated.h"

UCLASS()
class MESHCUTTING_API AUni_CuttingMeshes_Character : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AUni_CuttingMeshes_Character();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	//Variables
	float m_boxWidth = 5;

	FVector m_lastImpactPoint;
	FVector m_boxOrigin;

	FRotator m_boxRotation;
	UPROPERTY(EditAnywhere, Category = "Mesh Cutting")
	bool m_isCutting = false;
	UPROPERTY(EditAnywhere, Category = "Mesh Cutting")
	bool m_hitActorCutable = false;
	UPROPERTY(EditAnywhere, Category = "Mesh Cutting")
	bool m_holding = false;
	UPROPERTY(BlueprintReadOnly, Category = "Mesh Cutting")
	bool m_pickedUp = false;
	//Lists
	TArray<UProceduralMeshComponent*> m_cutMeshes;
	//TArray<FVector>	m_cutMeshesOrigin;
	//TArray<FRotator> m_cutMeshesRoation;
	TArray<AActor> m_lastHitActor;

	//TArray<UProceduralMeshComponent*> otherCutProcMeshes;
	//open and closing gate
	bool m_gateOpen = false;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void Start_Cutting();
	void Stop_Cutting();
	void SetUpCutting();
	void SetUpDebug();
	void ReturnToOriginalPosition();
	//UFUNCTION(BlueprintCallable, category = "MyBlueprintLibary")
	//void PickedUp(AActor* attackToComponent);

	UPROPERTY(BlueprintReadOnly, Category = "Mesh Cutting")
	UBoxComponent* box;
	UPROPERTY(EditAnywhere, Category = "Components")
	UActorComponent* hitComponent;
	UPROPERTY(EditAnywhere, Category = "Mesh Cutting")
	FString tag = "Grabbable";

};
