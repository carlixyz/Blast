// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "../Character/BlasterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "BulletShell.h"
#include "Engine/SkeletalMeshSocket.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(	ECollisionChannel::ECC_Pawn, 
											  ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if ( HasAuthority() ) // (GetLocalRole() == ENetRole::ROLE_Authority)
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn,
												  ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap); 
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap); 
		///  We bind these last functions only on server side
	}

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
	
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor,
							  UPrimitiveComponent* otherComponent, int32 otherBodyIndex, 
								bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* blasterCharacter = Cast<ABlasterCharacter>(otherActor);
	if (blasterCharacter)
	{
		blasterCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor,
								 UPrimitiveComponent* otherComponent, int32 otherBodyIndex)
{
	ABlasterCharacter* blasterCharacter = Cast<ABlasterCharacter>(otherActor);
	if (blasterCharacter)
	{
		blasterCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);	//	--Ammo;
	//SetHUDAmmo();
}

void AWeapon::OnRep_Ammo()
{
	//SetHUDAmmo();
}

void AWeapon::SetWeaponState(EWeaponState state)
{
	WeaponState = state;

	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:

		ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	}

}

void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
		case EWeaponState::EWS_Equipped:
			ShowPickupWidget(false);
			AreaSphere->SetCollisionEnabled( ECollisionEnabled::NoCollision );
			break;
	}
}



void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::Fire(const FVector& hitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}

	if (CasingClass)
	{
		const USkeletalMeshSocket* ammoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));

		if (ammoEjectSocket)
		{
			FTransform socketTransform = ammoEjectSocket->GetSocketTransform(WeaponMesh);

			UWorld* world = GetWorld();
			if (world)
			{
				world->SpawnActor<ABulletShell>(
					CasingClass,
					socketTransform.GetLocation(),
					socketTransform.GetRotation().Rotator());

			}
		}
	}
}

