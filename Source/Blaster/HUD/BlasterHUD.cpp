// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D viewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(viewportSize);
		const FVector2D viewportCenter(viewportSize.X / 2.f, viewportSize.Y / 2.f);
		float spreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		if (HUDPackage.CrosshairsCenter)
		{
			DrawCrosshair(HUDPackage.CrosshairsCenter, viewportCenter, FVector2D::ZeroVector, HUDPackage.CrosshairColor);
		}

		if (HUDPackage.CrosshairsLeft)
		{
			FVector2D spread(-spreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsLeft, viewportCenter, spread, HUDPackage.CrosshairColor);
		}

		if (HUDPackage.CrosshairsRight)
		{
			FVector2D spread(+spreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsRight, viewportCenter, spread, HUDPackage.CrosshairColor);
		}

		if (HUDPackage.CrosshairsTop)
		{
			FVector2D spread(0.f, -spreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsTop, viewportCenter, spread, HUDPackage.CrosshairColor);
		}

		if (HUDPackage.CrosshairsBottom)
		{
			FVector2D spread(0.f, +spreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsBottom, viewportCenter, spread, HUDPackage.CrosshairColor);
		}
	}
}

void ABlasterHUD::DrawCrosshair(UTexture2D* texture, FVector2D viewportCenter, FVector2D spread, FLinearColor crosshairColor)
{
	const float textureWidth = texture->GetSizeX();
	const float textureHeight = texture->GetSizeY();

	const FVector2D textureDrawPoint(
		viewportCenter.X - (textureWidth / 2.f) + spread.X,
		viewportCenter.Y - (textureHeight / 2.f) + spread.Y
	);

	DrawTexture( texture,
				textureDrawPoint.X,
				textureDrawPoint.Y,
				textureWidth,
				textureHeight,
				0.f,
				0.f,
				1.f,
				1.f,
				crosshairColor);
}
