// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& hitTarget)
{
	Super::Fire(hitTarget);

	if (!HasAuthority()) return;

	APawn* instigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* muzzleFlashSocket = 
		GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));

	if (muzzleFlashSocket)
	{
		FTransform socketTransform = muzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		/// From muzzle flash socket to hit location from TraceUnderCrosshair orientation
		FVector toTarget = hitTarget - socketTransform.GetLocation();
		FRotator targetRotation = toTarget.Rotation();

		if (instigatorPawn && ProjectileClass)
		{
			FActorSpawnParameters spawnParams;
			spawnParams.Owner = GetOwner();
			spawnParams.Instigator = instigatorPawn;
			UWorld* world = GetWorld();
			if (world)
			{
				world->SpawnActor<AProjectile>(
					ProjectileClass,
					socketTransform.GetLocation(),
					targetRotation,
					spawnParams );

			}
		}
	}
}
