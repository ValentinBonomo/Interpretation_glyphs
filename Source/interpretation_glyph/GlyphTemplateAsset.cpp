#include "GlyphTemplateAsset.h"
#include "Engine/Texture2D.h"

void UGlyphTemplateAsset::ExtractPointsFromImage()
{
	if (!SourceImage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GlyphTemplate] Pas d'image source assignee !"));
		return;
	}

	SourceImage->CompressionSettings = TC_VectorDisplacementmap;
	SourceImage->MipGenSettings = TMGS_NoMipmaps;
	SourceImage->SRGB = false;
	SourceImage->UpdateResource();

	FTexture2DMipMap& Mip = SourceImage->GetPlatformData()->Mips[0];
	const int32 Width = Mip.SizeX;
	const int32 Height = Mip.SizeY;

	const void* RawData = Mip.BulkData.Lock(LOCK_READ_ONLY);
	if (!RawData)
	{
		UE_LOG(LogTemp, Error, TEXT("[GlyphTemplate] Impossible de lire les pixels de l'image."));
		Mip.BulkData.Unlock();
		return;
	}

	const FColor* Pixels = static_cast<const FColor*>(RawData);

	TArray<FVector2D> DetectedPoints;
	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			const FColor& Pixel = Pixels[Y * Width + X];
			uint8 Brightness = FMath::Max3(Pixel.R, Pixel.G, Pixel.B);
			if (Brightness >= PixelThreshold)
			{
				DetectedPoints.Add(FVector2D(X, Y));
			}
		}
	}

	Mip.BulkData.Unlock();

	if (DetectedPoints.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GlyphTemplate] Aucun pixel detecte au-dessus du seuil %d dans '%s'"),
			PixelThreshold, *SourceImage->GetName());
		return;
	}

	const int32 MaxPoints = 200;
	if (DetectedPoints.Num() > MaxPoints)
	{
		TArray<FVector2D> Sampled;
		Sampled.Reserve(MaxPoints);
		float Step = (float)DetectedPoints.Num() / (float)MaxPoints;
		for (int32 i = 0; i < MaxPoints; ++i)
		{
			int32 Index = FMath::FloorToInt(i * Step);
			Sampled.Add(DetectedPoints[Index]);
		}
		DetectedPoints = MoveTemp(Sampled);
	}

	RawPoints = MoveTemp(DetectedPoints);
	MarkPackageDirty();

	UE_LOG(LogTemp, Log, TEXT("[GlyphTemplate] %d points extraits de '%s' (image %dx%d, seuil %d)"),
		RawPoints.Num(), *SourceImage->GetName(), Width, Height, PixelThreshold);
}
