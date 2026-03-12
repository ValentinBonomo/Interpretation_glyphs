#include "GlyphPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "GlyphDrawingWidget.h"
#include "GlyphTemplateAsset.h"

AGlyphPlayerController::AGlyphPlayerController()
{
}

void AGlyphPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (GlyphMappingContext)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(GlyphMappingContext, 0);
		}
	}
}

void AGlyphPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (ToggleDrawingAction)
		{
			EIC->BindAction(ToggleDrawingAction, ETriggerEvent::Started, this, &AGlyphPlayerController::ToggleDrawingWidget);
		}
	}
}

void AGlyphPlayerController::ToggleDrawingWidget()
{
	if (bWidgetVisible)
	{
		HideDrawingWidget();
	}
	else
	{
		ShowDrawingWidget();
	}
}

void AGlyphPlayerController::ShowDrawingWidget()
{
	if (!DrawingWidgetClass)
	{
		return;
	}

	if (!DrawingWidget)
	{
		DrawingWidget = CreateWidget<UGlyphDrawingWidget>(this, DrawingWidgetClass);
		if (!DrawingWidget)
		{
			return;
		}

		DrawingWidget->GlyphTemplates = GlyphTemplates;
		DrawingWidget->GlyphSpawnLocation = GlyphSpawnLocation;
	}

	DrawingWidget->AddToViewport();
	bWidgetVisible = true;

	bShowMouseCursor = true;
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void AGlyphPlayerController::HideDrawingWidget()
{
	if (DrawingWidget && DrawingWidget->IsInViewport())
	{
		DrawingWidget->RemoveFromParent();
	}

	bWidgetVisible = false;

	bShowMouseCursor = false;
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
}
