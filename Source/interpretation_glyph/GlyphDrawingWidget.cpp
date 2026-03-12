#include "GlyphDrawingWidget.h"
#include "Components/Image.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/Canvas.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

void UGlyphDrawingWidget::NativeConstruct()
{
	Super::NativeConstruct();
	InitRenderTarget();
}

void UGlyphDrawingWidget::InitRenderTarget()
{
	RenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(
		this,
		RenderTargetSize,
		RenderTargetSize,
		ETextureRenderTargetFormat::RTF_RGBA8
	);

	if (!RenderTarget || !DrawingDisplay)
	{
		return;
	}

	ClearDrawing();

	FSlateBrush NewBrush;
	NewBrush.SetResourceObject(RenderTarget);
	NewBrush.ImageSize = FVector2D(RenderTargetSize, RenderTargetSize);
	DrawingDisplay->SetBrush(NewBrush);
}

void UGlyphDrawingWidget::ClearDrawing()
{
	if (RenderTarget)
	{
		UKismetRenderingLibrary::ClearRenderTarget2D(this, RenderTarget, BackgroundColor);
	}
}

FReply UGlyphDrawingWidget::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDrawing = true;
		CurrentStrokePoints.Empty();
		ClearDrawing();

		FVector2D RTPos = ToRTSpace(InGeometry, InMouseEvent.GetScreenSpacePosition());
		CurrentStrokePoints.Add(RTPos);
		LastRTPoint = RTPos;

		return FReply::Handled().CaptureMouse(GetCachedWidget().ToSharedRef());
	}

	return FReply::Unhandled();
}

FReply UGlyphDrawingWidget::NativeOnMouseMove(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (!bIsDrawing)
	{
		return FReply::Unhandled();
	}

	FVector2D RTPos = ToRTSpace(InGeometry, InMouseEvent.GetScreenSpacePosition());

	if (FVector2D::Distance(RTPos, LastRTPoint) >= MinPointDistance)
	{
		DrawLine(LastRTPoint, RTPos);
		CurrentStrokePoints.Add(RTPos);
		LastRTPoint = RTPos;
	}

	return FReply::Handled();
}

FReply UGlyphDrawingWidget::NativeOnMouseButtonUp(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bIsDrawing)
	{
		bIsDrawing = false;
		FinalizeStroke();

		return FReply::Handled().ReleaseMouseCapture();
	}

	return FReply::Unhandled();
}

void UGlyphDrawingWidget::DrawLine(FVector2D FromRT, FVector2D ToRT)
{
	if (!RenderTarget)
	{
		return;
	}

	FDrawToRenderTargetContext Context;
	UCanvas* Canvas = nullptr;
	FVector2D Size;

	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, RenderTarget, Canvas, Size, Context);

	if (Canvas)
	{
		Canvas->K2_DrawLine(FromRT, ToRT, StrokeThickness, StrokeColor);
	}

	UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, Context);
}

FVector2D UGlyphDrawingWidget::ToRTSpace(const FGeometry& Geometry, FVector2D AbsoluteScreenPos) const
{
	FVector2D LocalPos = Geometry.AbsoluteToLocal(AbsoluteScreenPos);
	FVector2D NormPos = LocalPos / Geometry.GetLocalSize();
	return NormPos * FVector2D(RenderTargetSize, RenderTargetSize);
}

void UGlyphDrawingWidget::FinalizeStroke()
{
	if (CurrentStrokePoints.Num() < 2)
	{
		return;
	}

	if (GlyphTemplates.Num() == 0)
	{
		return;
	}

	FGlyphRecognitionResult Result = UGlyphRecognizer::Recognize(CurrentStrokePoints, GlyphTemplates);

	UE_LOG(LogTemp, Log, TEXT("[GlyphDrawing] Score = %.4f | Reconnu = %s"),
		Result.Score,
		Result.bRecognized ? TEXT("OUI") : TEXT("NON"));

	if (Result.bRecognized && Result.BestMatch)
	{
		UE_LOG(LogTemp, Log, TEXT("[GlyphDrawing] Glyphe reconnu : %s"), *Result.BestMatch->GlyphName);

		UWorld* World = GetWorld();
		if (!World)
		{
			OnGlyphRecognized.Broadcast(Result.BestMatch);
			return;
		}

		FVector FinalSpawnLocation = GlyphSpawnLocation;
		APlayerController* PC = World->GetFirstPlayerController();
		if (PC && PC->GetPawn())
		{
			APawn* Pawn = PC->GetPawn();
			FinalSpawnLocation = Pawn->GetActorLocation() + Pawn->GetActorForwardVector() * 250.0f;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (Result.BestMatch->ActorToSpawn)
		{
			AActor* SpawnedActor = World->SpawnActor<AActor>(
				Result.BestMatch->ActorToSpawn,
				FinalSpawnLocation,
				FRotator::ZeroRotator,
				SpawnParams
			);

			if (SpawnedActor)
			{
				UE_LOG(LogTemp, Log, TEXT("[GlyphDrawing] Actor spawne : %s a %s"),
					*SpawnedActor->GetName(), *FinalSpawnLocation.ToString());
			}
		}
		else if (Result.BestMatch->MeshToSpawn)
		{
			AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>(
				AStaticMeshActor::StaticClass(),
				FinalSpawnLocation,
				FRotator::ZeroRotator,
				SpawnParams
			);

			if (MeshActor)
			{
				MeshActor->GetStaticMeshComponent()->SetStaticMesh(Result.BestMatch->MeshToSpawn);
				MeshActor->SetMobility(EComponentMobility::Movable);
				UE_LOG(LogTemp, Log, TEXT("[GlyphDrawing] Mesh spawne : %s a %s"),
					*Result.BestMatch->MeshToSpawn->GetName(), *FinalSpawnLocation.ToString());
			}
		}

		if (Result.BestMatch->SpawnVFX)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World,
				Result.BestMatch->SpawnVFX,
				FinalSpawnLocation,
				FRotator::ZeroRotator,
				FVector(1.0f),
				true,
				true,
				ENCPoolMethod::None
			);
		}

		OnGlyphRecognized.Broadcast(Result.BestMatch);
	}
	else
	{
		OnGlyphFailed.Broadcast();
	}
}
