#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GlyphPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class UGlyphDrawingWidget;
class UGlyphTemplateAsset;

UCLASS()
class INTERPRETATION_GLYPH_API AGlyphPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	AGlyphPlayerController();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Glyph|Input")
	UInputAction* ToggleDrawingAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Glyph|Input")
	UInputMappingContext* GlyphMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Glyph|Widget")
	TSubclassOf<UGlyphDrawingWidget> DrawingWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Glyph|Widget")
	TArray<UGlyphTemplateAsset*> GlyphTemplates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glyph|Spawn")
	FVector GlyphSpawnLocation = FVector::ZeroVector;

protected:

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:

	UPROPERTY()
	UGlyphDrawingWidget* DrawingWidget = nullptr;

	bool bWidgetVisible = false;

	void ToggleDrawingWidget();
	void ShowDrawingWidget();
	void HideDrawingWidget();
};
