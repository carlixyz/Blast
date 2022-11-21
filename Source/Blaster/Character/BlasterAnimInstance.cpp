// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/EnumTypes/CombatState.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

void UBlasterAnimInstance::NativeUpdateAnimation(float deltaTime)
{
	Super::NativeUpdateAnimation(deltaTime);

	if (BlasterCharacter == nullptr)
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());

	}
	if (BlasterCharacter == nullptr)
		return;

	FVector Velocity = BlasterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = (BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f);
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
	EquippedWeapon = BlasterCharacter->GetEquippedWeapon();
	bIsCrouched = BlasterCharacter->bIsCrouched;
	bIsAiming = BlasterCharacter->IsAiming();
	TurningInPlace = BlasterCharacter->GetTurningInPlace();
	bRotateRootBone = BlasterCharacter->ShouldRotateRootBone();
	bElimined = BlasterCharacter->IsElimined();

	/// Offset yaw for Strafing
	FRotator aimRotation = BlasterCharacter->GetBaseAimRotation();
	FRotator movementeRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());
	FRotator deltaRot = UKismetMathLibrary::NormalizedDeltaRotator(movementeRotation,
																   aimRotation);

	DeltaRotation = FMath::RInterpTo(DeltaRotation, deltaRot, deltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = BlasterCharacter->GetActorRotation();

	const FRotator delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation,
																	  CharacterRotationLastFrame);
	const float target = delta.Yaw / deltaTime;
	const float interp = FMath::FInterpTo(Lean, target, deltaTime, 6.f);
	Lean = FMath::Clamp(interp, -90.f, 90.f);

	AO_Yaw = BlasterCharacter->GetAO_Yaw();
	AO_Pitch = BlasterCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon &&
		EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh())
	{
		LeftHandTransform =
			EquippedWeapon->GetWeaponMesh()->GetSocketTransform(
				FName("LeftHandSocket"),
				ERelativeTransformSpace::RTS_World);

		FVector outPosition;
		FRotator outRotation;
		BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"),
														  LeftHandTransform.GetLocation(),
														  FRotator::ZeroRotator,
														  outPosition,
														  outRotation);
		LeftHandTransform.SetLocation(outPosition);
		LeftHandTransform.SetRotation(FQuat(outRotation));

		if (BlasterCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform rightHandTransform =
				EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("hand_r"),
																	ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(rightHandTransform.GetLocation(),
																			 rightHandTransform.GetLocation() +
																			 (rightHandTransform.GetLocation() -
																			  BlasterCharacter->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, deltaTime, 30.f);

			/*
			FTransform muzzleTipTransform =
				EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"),
																	ERelativeTransformSpace::RTS_World);
			FVector muzzleX(FRotationMatrix(muzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
			DrawDebugLine(	GetWorld(), muzzleTipTransform.GetLocation(),
						  muzzleTipTransform.GetLocation() + muzzleX * 1000.f, FColor::Red);
			DrawDebugLine(	GetWorld(), muzzleTipTransform.GetLocation(),
						  BlasterCharacter->GetHitTarget(), FColor::Orange);
			*/
		}
	}

	bUseFABRIK = BlasterCharacter->GetCombatState() != ECombatState::ECS_Reloading;
	bUseAimOffsets = BlasterCharacter->GetCombatState() != ECombatState::ECS_Reloading;
	bTransformRightHand = BlasterCharacter->GetCombatState() != ECombatState::ECS_Reloading;
}
