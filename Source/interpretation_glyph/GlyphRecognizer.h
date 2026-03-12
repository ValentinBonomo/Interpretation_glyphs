#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GlyphTemplateAsset.h"
#include "GlyphRecognizer.generated.h"

USTRUCT(BlueprintType)
struct FGlyphRecognitionResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Glyph")
	UGlyphTemplateAsset* BestMatch = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Glyph")
	float Score = TNumericLimits<float>::Max();

	UPROPERTY(BlueprintReadOnly, Category = "Glyph")
	bool bRecognized = false;
};

UCLASS()
class INTERPRETATION_GLYPH_API UGlyphRecognizer : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Glyph Recognizer")
	static FGlyphRecognitionResult Recognize(
		const TArray<FVector2D>& InputPoints,
		const TArray<UGlyphTemplateAsset*>& Templates);

	UFUNCTION(BlueprintCallable, Category = "Glyph Recognizer|Preprocessing")
	static TArray<FVector2D> Resample(const TArray<FVector2D>& Points, int32 N);

	UFUNCTION(BlueprintCallable, Category = "Glyph Recognizer|Preprocessing")
	static TArray<FVector2D> ScaleToUnitSquare(const TArray<FVector2D>& Points);

	UFUNCTION(BlueprintCallable, Category = "Glyph Recognizer|Preprocessing")
	static TArray<FVector2D> TranslateToOrigin(const TArray<FVector2D>& Points);

	UFUNCTION(BlueprintCallable, Category = "Glyph Recognizer|Preprocessing")
	static TArray<FVector2D> Preprocess(const TArray<FVector2D>& Points, int32 N);

	UFUNCTION(BlueprintCallable, Category = "Glyph Recognizer|Debug")
	static float PathLength(const TArray<FVector2D>& Points);

	UFUNCTION(BlueprintCallable, Category = "Glyph Recognizer|Debug")
	static FVector2D Centroid(const TArray<FVector2D>& Points);

private:

	static float GreedyCloudMatch(
		const TArray<FVector2D>& Sketch,
		const TArray<FVector2D>& Template,
		int32 N);

	static float CloudDistance(
		const TArray<FVector2D>& A,
		const TArray<FVector2D>& B,
		int32 Start,
		int32 N);
};
