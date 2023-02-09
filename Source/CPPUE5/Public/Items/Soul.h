// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Items.h"
#include "Soul.generated.h"

/**
 * 
 */
UCLASS()
class CPPUE5_API ASoul : public AItems
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category = "Soul Properties")
	int32 Souls;
	double DesiredZ;
	UPROPERTY(EditAnywhere, Category = "Soul Properties")
	float DriftRate = -15.f;

public:
	FORCEINLINE int32 GetSouls() const { return Souls; }
	FORCEINLINE void SetSouls(int32 NumberOfSouls) { Souls =  NumberOfSouls; }
};
