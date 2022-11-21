// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletShell.generated.h"

UCLASS()
class BLASTER_API ABulletShell : public AActor
{
	GENERATED_BODY()
	
public:	
	ABulletShell();

	FTimerHandle AutoDestroyTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AutoDestroyDelay;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* hitComp,
						   AActor* otherActor,
						   UPrimitiveComponent* otherComp,
						   FVector normalImpulse,
						   const FHitResult& hit);

	UFUNCTION()
	void Disappear();

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CasingMesh;

	UPROPERTY(EditAnywhere)
	float ShellEjectionImpulse;

	UPROPERTY(EditAnywhere)
	class USoundCue* ShellSound;

	bool ShellSoundPlayed;
};
