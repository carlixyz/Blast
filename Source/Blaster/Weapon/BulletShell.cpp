// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletShell.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"

ABulletShell::ABulletShell()
{
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);
	CasingMesh->SetCollisionResponseToChannel(	ECollisionChannel::ECC_Camera, 
												ECollisionResponse::ECR_Ignore);
	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
	CasingMesh->SetNotifyRigidBodyCollision(true);
	ShellEjectionImpulse = 5.f;
	AutoDestroyDelay = 1.0f;
	ShellSoundPlayed = false;
}

void ABulletShell::BeginPlay()
{
	Super::BeginPlay();

	CasingMesh->OnComponentHit.AddDynamic(this, &ABulletShell::OnHit);
	//CasingMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);
	FVector randomShell = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(GetActorForwardVector(), 20.f);
	CasingMesh->AddImpulse(randomShell * ShellEjectionImpulse);
}

void ABulletShell::OnHit(UPrimitiveComponent* hitComp, AActor* otherActor, UPrimitiveComponent* otherComp, FVector normalImpulse, const FHitResult& hit)
{
	if (ShellSound && !ShellSoundPlayed)
	{
		ShellSoundPlayed = true;
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
		GetWorldTimerManager().SetTimer(AutoDestroyTimer, this, &ABulletShell::Disappear, AutoDestroyDelay);
	}
}

void ABulletShell::Disappear()
{
	Destroy();
}

