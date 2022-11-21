
#include "CombatComponent.h"
#include "../Weapon/Weapon.h"
#include "../Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

		auto followCamera = Character->GetFollowCamera();
		if (followCamera)
		{
			DefaultFOV = followCamera->FieldOfView;
			CurrentFOV = DefaultFOV;
		}

		//if (Character->HasAuthority())
		//{
		//	InitializeCarriedAmmo();
		//}
	}

}

void UCombatComponent::TickComponent(float deltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(deltaTime, TickType, ThisTickFunction);

	if (Character && Character->IsLocallyControlled())
	{
		FHitResult hitResult;
		TraceUnderCrosshairs(hitResult);
		HitTarget = hitResult.ImpactPoint;

		SetHUDCrosshairs(deltaTime);
		InterpFOV(deltaTime);
	}

}

void UCombatComponent::SetHUDCrosshairs(float deltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr)
		return;

	Controller = Controller == nullptr ?
		Cast<ABlasterPlayerController>(Character->Controller) :
		Controller;

	if (Controller)
	{
		HUD = (HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD);
		if (HUD)
		{
			if (EquippedWeapon)
			{
				HUDPack.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPack.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPack.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPack.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				HUDPack.CrosshairsTop = EquippedWeapon->CrosshairsTop;
			}
			else
			{
				HUDPack.CrosshairsCenter = nullptr;
				HUDPack.CrosshairsLeft = nullptr;
				HUDPack.CrosshairsRight = nullptr;
				HUDPack.CrosshairsBottom = nullptr;
				HUDPack.CrosshairsTop = nullptr;
			}

			// Calculate crosshair spread [0, 600] -> [0, 1]
			FVector2D walkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D velocityMultiplierRange(0.f, 1.f);
			FVector velocity = Character->GetVelocity();
			velocity.Z = 0.f;

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(walkSpeedRange,
																		velocityMultiplierRange,
																		velocity.Size());
			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, deltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, deltaTime, 30.f);
			}

			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.6f, deltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, deltaTime, 30.f);
			}

			CrosshairShootFactor = FMath::FInterpTo(CrosshairShootFactor, 0.f, deltaTime, 40.f);

			HUDPack.CrosshairSpread = 0.5f +
				CrosshairVelocityFactor +
				CrosshairInAirFactor -
				CrosshairAimFactor +
				CrosshairShootFactor;

			HUD->SetHUDPackage(HUDPack);
		}
	}
}

void UCombatComponent::HandleReload()
{
	//Character->PlayReloadMontage();
}


void UCombatComponent::InterpFOV(float deltaTime)
{
	if (EquippedWeapon == nullptr)
		return;

	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV,
									  EquippedWeapon->GetZoomedFOV(),
									  deltaTime,
									  EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV,
									  DefaultFOV,
									  deltaTime,
									  ZoomInterpSpeed);
	}

	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;

	case ECombatState::ECS_Unoccupied:
	{
		if (bFireButtonPressed)
			Fire();
	}
	break;
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed =
			bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed =
			bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}


//--------------------------------------------------------------------------

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::Fire()
{
	if (bCanFire)
	{
		//bCanFire = false;
		ServerFire(HitTarget); /// This calls ServerFire_Implementation

		if (EquippedWeapon)
		{
			bCanFire = false; // And add it back here !!!!!!!
			CrosshairShootFactor = 0.75f;
		}
		StartFireTimer();
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& traceHitTarget)
{
	MulticastFire(traceHitTarget); /// Calls MulticastFire
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& traceHitTarget)
{
	if (EquippedWeapon == nullptr)
		return;

	if (Character)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(traceHitTarget);
	}
}


void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr)
		return;

	//bCanFire = false;
	Character->GetWorldTimerManager().SetTimer(FireTimer,
											   this,
											   &UCombatComponent::FireTimerFinished,
											   EquippedWeapon->FireDelay);
}
void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr)
		return;

	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}

	//if (EquippedWeapon->IsEmpty())
	//{
	//	Reload();
	//}
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr)
		return false;

	return (!EquippedWeapon->IsEmpty() && bCanFire &&
			CombatState == ECombatState::ECS_Unoccupied);
}

//--------------------------------------------------------------------------


void UCombatComponent::TraceUnderCrosshairs(FHitResult& traceHitResult)
{
	FVector2D viewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(viewportSize);
	}

	FVector2D crosshairLocation(viewportSize.X / 2.0f, viewportSize.Y / 2.0f);
	FVector crosshairWorldPosition;
	FVector crosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		crosshairLocation,
		crosshairWorldPosition,
		crosshairWorldDirection);


	if (bScreenToWorld)
	{
		FVector start{ crosshairWorldPosition };

		if (Character)
		{
			float distanceToCharacter = (Character->GetActorLocation() - start).Size();
			start += crosshairWorldDirection * (distanceToCharacter + 100.f);
		}

		FVector end{ start + crosshairWorldDirection * TRACE_LENGTH };

		if (!GetWorld()->LineTraceSingleByChannel(traceHitResult,
												  start,
												  end,
												  ECollisionChannel::ECC_Visibility))
			traceHitResult.ImpactPoint = end;

		/// Check if there's an actor & then use the interface as a filter to check if it's a BlasterCharacter
		if (traceHitResult.GetActor() &&
			traceHitResult.GetActor()->Implements<UInteractionCrosshairInterface>())
		{
			HUDPack.CrosshairColor = FLinearColor::Red;
		}
		else
		{
			HUDPack.CrosshairColor = FLinearColor::White;
		}

		/*
		if (!traceHitResult.bBlockingHit)
		{
			traceHitResult.ImpactPoint = end;
			HitTarget = end;
		}
		else
		{
			HitTarget = traceHitResult.ImpactPoint;
		//DrawDebugSphere(GetWorld(),
		//					traceHitResult.ImpactPoint,
		//					12.f,
		//					12.f,
		//					FColor::Red);
		}
		*/

	}
}

void UCombatComponent::EquipWeapon(AWeapon* weaponToEquip)
{
	if (Character == nullptr || weaponToEquip == nullptr)
		return;

	EquippedWeapon = weaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	const USkeletalMeshSocket* handSocket =
		Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (handSocket)
	{
		handSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}

	EquippedWeapon->SetOwner(Character);
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::Reload()
{
}

