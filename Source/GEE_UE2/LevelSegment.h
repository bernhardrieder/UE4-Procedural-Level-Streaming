// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <functional>
#include "GameFramework/Actor.h"
#include "LevelSegment.generated.h"

UENUM(BlueprintType)
enum class ELevelSegmentTypes : uint8
{
	LST_Corridor 	UMETA(DisplayName = "Corridor"),
	LST_Cross 		UMETA(DisplayName = "Cross"),
	LST_Curve		UMETA(DisplayName = "Curve"),
	LST_StartEnd	UMETA(DisplayName = "SartEnd"),
	LST_T			UMETA(DisplayName = "T"),
	LST_None		UMETA(DisplayName = "None")
};

UCLASS()
class GEE_UE2_API ALevelSegment : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Level Segment")
	ELevelSegmentTypes Type;

	UPROPERTY(EditDefaultsOnly, Category = "Level Segment")
	int SegmentWidth;

	UPROPERTY(EditDefaultsOnly, Category = "Level Segment")
	UBoxComponent* BoxCollider;

public:	
	ALevelSegment();

	UFUNCTION(BlueprintCallable, Category = "Level Segment")
	FVector GetNeighbourPosition(int doorIndex);

	UFUNCTION(BlueprintCallable, Category = "Level Segment")
	void SetTriggerComponent(UBoxComponent* trigger);

	UFUNCTION(BlueprintCallable, Category = "Level Segment")
	void AddDoor(USceneComponent* door);

	UFUNCTION(BlueprintCallable, Category = "Level Segment")
	void AddDoors(TArray<USceneComponent*> doors_array);

	UFUNCTION(BlueprintCallable, Category = "Level Segment")
	TArray<USceneComponent*> GetDoors() const;

	bool IsNeeded() const;
	void IsNeeded(bool val);
	struct LevelSegmentProperties ToLevelSegmentProperties() const;

	class AWorldManager* WorldManager;
	TArray<int> NeighbourIndexes;
private:

	bool m_isNeeded = false;	
	TArray<USceneComponent*> m_doors;

protected:
	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
