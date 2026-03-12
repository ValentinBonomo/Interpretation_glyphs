#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GlyphTemplateAsset.h"
#include "GlyphRecognizer.h"
#include "GlyphDrawingWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGlyphRecognized, UGlyphTemplateAsset*, RecognizedTemplate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGlyphFailed);

UCLASS()
class INTERPRETATION_GLYPH_API UGlyphDrawingWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	class UImage* DrawingDisplay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glyph")
	TArray<UGlyphTemplateAsset*> GlyphTemplates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glyph")
	FVector GlyphSpawnLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glyph|Dessin", meta = (ClampMin = "64", ClampMax = "1024"))
	int32 RenderTargetSize = 512;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glyph|Dessin", meta = (ClampMin = "1.0"))
	float StrokeThickness = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glyph|Dessin")
	FLinearColor StrokeColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glyph|Dessin")
	FLinearColor BackgroundColor = FLinearColor::Black;

	UPROPERTY(BlueprintAssignable, Category = "Glyph")
	FOnGlyphRecognized OnGlyphRecognized;

	UPROPERTY(BlueprintAssignable, Category = "Glyph")
	FOnGlyphFailed OnGlyphFailed;

	UFUNCTION(BlueprintCallable, Category = "Glyph")
	void ClearDrawing();

	UFUNCTION(BlueprintCallable, Category = "Glyph")
	const TArray<FVector2D>& GetLastStrokePoints() const { return CurrentStrokePoints; }

protected:

	virtual void NativeConstruct() override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:

	UPROPERTY()
	class UTextureRenderTarget2D* RenderTarget = nullptr;

	TArray<FVector2D> CurrentStrokePoints;
	FVector2D LastRTPoint;
	bool bIsDrawing = false;
	static constexpr float MinPointDistance = 3.0f;

	void InitRenderTarget();
	void DrawLine(FVector2D FromRT, FVector2D ToRT);
	FVector2D ToRTSpace(const FGeometry& Geometry, FVector2D AbsoluteScreenPos) const;
	void FinalizeStroke();
};
