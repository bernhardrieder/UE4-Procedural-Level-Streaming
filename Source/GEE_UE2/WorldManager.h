// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "LevelGenerator.h"
#include "LevelSegment.h"
#include "GameFramework/Actor.h"
#include "WorldManager.generated.h"

USTRUCT()
struct FLevelSegmentMapping
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	ELevelSegmentTypes Type;

	UPROPERTY(EditAnywhere)
	FString MapName;

	FLevelSegmentMapping () : FLevelSegmentMapping(ELevelSegmentTypes::LST_None, "") {}
	FLevelSegmentMapping (ELevelSegmentTypes type, FString name) : Type(type), MapName(name) {}
};

UCLASS()
class GEE_UE2_API AWorldManager : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	class ALevelGenerator* LevelGenerator;

	UPROPERTY(EditAnywhere)
	FString MapFolderName;

	UPROPERTY(EditAnywhere)
	TArray<FLevelSegmentMapping> LevelSegmentMapping;

public:	
	// Sets default values for this actor's properties
	AWorldManager();

	virtual void BeginPlay() override;

	void LoadLevelSegment(const LevelSegmentProperties& properties);
	void LoadLevelSegments(ALevelSegment* root, const TArray<int>& neighbourIndexes);
private:
	TArray<LevelSegmentProperties> m_generatedLevel;
	TArray<FVector> m_usedLocations;
	TArray<ALevelSegment*> m_loadedLevelSegments;
	TMap<ELevelSegmentTypes, FString> m_levelSegmentMapping;
	TMap<ELevelSegmentTypes, int> m_levelSegmentTypeCounter;
	TArray<TPair<ULevelStreaming*, LevelSegmentProperties>> m_newLoadedLevels;
	class std::function<void(void)> m_additionalSetActiveLevelSegmentAction = nullptr;

	ULevelStreaming* loadLevelInstance(UObject* WorldContextObject, FString MapFolderOffOfContent, FString LevelName, int32 InstanceNumber, FVector Location, FRotator Rotation, bool& Success) const;
	void initalizeLevelSegmentListsAndDictionaries();
	void loadWorldRootSegment(const LevelSegmentProperties& properties);
	void unloadUnneededMaps();
	bool isLocationInUse(const FVector& location);
	int findUsedLocationIndex(const FVector& location);
	void removeUsedLocation(const FVector& location);

	UFUNCTION()
	void setActiveLevelSegments();
};
