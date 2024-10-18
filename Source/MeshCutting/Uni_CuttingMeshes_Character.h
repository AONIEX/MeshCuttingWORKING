// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "ProceduralMeshComponent.h"
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
	float boxWidth;

	FVector lastImpactPoint;
	FVector boxOrigin;

	FRotator boxRotation;
	UPROPERTY(EditAnywhere, Category = "Mesh Cutting")
	bool isCutting = true;
	UPROPERTY(EditAnywhere, Category = "Mesh Cutting")
	bool hitActorCutable;
	UPROPERTY(EditAnywhere, Category = "Mesh Cutting")
	bool holding;
	UPROPERTY(BlueprintReadOnly, Category = "Mesh Cutting")
	bool pickedUp = false;
	//Lists
	TArray<UProceduralMeshComponent> cutMeshes;
	TArray<FVector>	cutMeshesOrigin;
	TArray<FRotator> cutMeshesRoation;
	TArray<AActor> lastHitActor;

	//open and closing gate
	bool gateOpen = false;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void Start_Cutting();
	void Stop_Cutting();
	void SetUpCutting();
	void SetUpDebug();
	UFUNCTION(BlueprintCallable, category = "MyBlueprintLibary")
	void PickedUp(AActor* attackToComponent);

	UPROPERTY(BlueprintReadOnly, Category = "Mesh Cutting")
	UBoxComponent* box;
	UPROPERTY(EditAnywhere, Category = "Components")
	UActorComponent* hitComponent;


};
