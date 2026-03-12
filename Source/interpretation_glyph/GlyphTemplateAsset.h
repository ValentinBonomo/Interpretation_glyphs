#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "NiagaraSystem.h"
#include "GlyphTemplateAsset.generated.h"

UCLASS(BlueprintType)
class INTERPRETATION_GLYPH_API UGlyphTemplateAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glyph")
	FString GlyphName = TEXT("MonGlyphe");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glyph|Image")
	UTexture2D* SourceImage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glyph|Image", meta = (ClampMin = "1", ClampMax = "255"))
	uint8 PixelThreshold = 128;

	UFUNCTION(CallInEditor, Category = "Glyph|Image")
	void ExtractPointsFromImage();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glyph")
	TArray<FVector2D> RawPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glyph|Parametres", meta = (ClampMin = "8", ClampMax = "128"))
	int32 NumPoints = 32;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glyph|Parametres", meta = (ClampMin = "0.0"))
	float MatchThreshold = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glyph|Spawn")
	TSubclassOf<AActor> ActorToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glyph|Spawn")
	UStaticMesh* MeshToSpawn = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glyph|VFX")
	UNiagaraSystem* SpawnVFX = nullptr;
};
