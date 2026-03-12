#include "GlyphRecognizer.h"

FGlyphRecognitionResult UGlyphRecognizer::Recognize(
	const TArray<FVector2D>& InputPoints,
	const TArray<UGlyphTemplateAsset*>& Templates)
{
	FGlyphRecognitionResult Result;

	if (InputPoints.Num() < 2 || Templates.Num() == 0)
	{
		return Result;
	}

	for (UGlyphTemplateAsset* Template : Templates)
	{
		if (!Template || Template->RawPoints.Num() < 2)
		{
			continue;
		}

		const int32 N = Template->NumPoints;

		TArray<FVector2D> SketchProcessed = Preprocess(InputPoints, N);
		TArray<FVector2D> TemplateProcessed = Preprocess(Template->RawPoints, N);

		float Score = GreedyCloudMatch(SketchProcessed, TemplateProcessed, N);

		if (Score < Result.Score)
		{
			Result.Score = Score;
			Result.BestMatch = Template;
		}
	}

	if (Result.BestMatch && Result.Score < Result.BestMatch->MatchThreshold)
	{
		Result.bRecognized = true;
	}

	return Result;
}

TArray<FVector2D> UGlyphRecognizer::Preprocess(const TArray<FVector2D>& Points, int32 N)
{
	TArray<FVector2D> Result = Resample(Points, N);
	Result = ScaleToUnitSquare(Result);
	Result = TranslateToOrigin(Result);
	return Result;
}

TArray<FVector2D> UGlyphRecognizer::Resample(const TArray<FVector2D>& InPoints, int32 N)
{
	if (InPoints.Num() < 2 || N < 2)
	{
		return InPoints;
	}

	TArray<FVector2D> Points = InPoints;

	const float TotalLen = PathLength(Points);
	if (TotalLen <= 0.0f)
	{
		TArray<FVector2D> Result;
		Result.Init(Points[0], N);
		return Result;
	}

	const float Interval = TotalLen / (float)(N - 1);
	float CumulDist = 0.0f;

	TArray<FVector2D> Result;
	Result.Reserve(N);
	Result.Add(Points[0]);

	int32 i = 1;
	while (i < Points.Num() && Result.Num() < N)
	{
		const float d = FVector2D::Distance(Points[i - 1], Points[i]);

		if (CumulDist + d >= Interval)
		{
			const float t = (Interval - CumulDist) / d;
			const FVector2D q = Points[i - 1] + t * (Points[i] - Points[i - 1]);

			Result.Add(q);
			Points.Insert(q, i);
			i++;
			CumulDist = 0.0f;
		}
		else
		{
			CumulDist += d;
			i++;
		}
	}

	while (Result.Num() < N)
	{
		Result.Add(Points.Last());
	}

	return Result;
}

TArray<FVector2D> UGlyphRecognizer::ScaleToUnitSquare(const TArray<FVector2D>& Points)
{
	if (Points.Num() == 0)
	{
		return Points;
	}

	float MinX = TNumericLimits<float>::Max();
	float MaxX = TNumericLimits<float>::Lowest();
	float MinY = TNumericLimits<float>::Max();
	float MaxY = TNumericLimits<float>::Lowest();

	for (const FVector2D& P : Points)
	{
		MinX = FMath::Min(MinX, P.X);
		MaxX = FMath::Max(MaxX, P.X);
		MinY = FMath::Min(MinY, P.Y);
		MaxY = FMath::Max(MaxY, P.Y);
	}

	const float W = MaxX - MinX;
	const float H = MaxY - MinY;

	if (W <= KINDA_SMALL_NUMBER && H <= KINDA_SMALL_NUMBER)
	{
		return Points;
	}

	const float ScaleX = (W > KINDA_SMALL_NUMBER) ? W : H;
	const float ScaleY = (H > KINDA_SMALL_NUMBER) ? H : W;

	TArray<FVector2D> Result;
	Result.Reserve(Points.Num());

	for (const FVector2D& P : Points)
	{
		Result.Add(FVector2D(
			(P.X - MinX) / ScaleX,
			(P.Y - MinY) / ScaleY
		));
	}

	return Result;
}

TArray<FVector2D> UGlyphRecognizer::TranslateToOrigin(const TArray<FVector2D>& Points)
{
	if (Points.Num() == 0)
	{
		return Points;
	}

	const FVector2D Cent = Centroid(Points);

	TArray<FVector2D> Result;
	Result.Reserve(Points.Num());

	for (const FVector2D& P : Points)
	{
		Result.Add(P - Cent);
	}

	return Result;
}

float UGlyphRecognizer::PathLength(const TArray<FVector2D>& Points)
{
	float D = 0.0f;
	for (int32 i = 1; i < Points.Num(); i++)
	{
		D += FVector2D::Distance(Points[i - 1], Points[i]);
	}
	return D;
}

FVector2D UGlyphRecognizer::Centroid(const TArray<FVector2D>& Points)
{
	FVector2D Sum = FVector2D::ZeroVector;
	for (const FVector2D& P : Points)
	{
		Sum += P;
	}
	return Sum / (float)Points.Num();
}

float UGlyphRecognizer::GreedyCloudMatch(
	const TArray<FVector2D>& Sketch,
	const TArray<FVector2D>& Template,
	int32 N)
{
	const float e = 0.5f;
	const int32 Step = FMath::Max(1, FMath::FloorToInt(FMath::Pow((float)N, 1.0f - e)));

	float MinScore = TNumericLimits<float>::Max();

	for (int32 i = 0; i < N; i += Step)
	{
		const float d1 = CloudDistance(Sketch, Template, i, N);
		const float d2 = CloudDistance(Template, Sketch, i, N);
		MinScore = FMath::Min(MinScore, FMath::Min(d1, d2));
	}

	return MinScore;
}

float UGlyphRecognizer::CloudDistance(
	const TArray<FVector2D>& A,
	const TArray<FVector2D>& B,
	int32 Start,
	int32 N)
{
	TArray<bool> Matched;
	Matched.Init(false, N);

	float Sum = 0.0f;

	for (int32 Count = 0; Count < N; Count++)
	{
		const int32 Idx = (Start + Count) % N;

		int32 ClosestJ = -1;
		float MinDist = TNumericLimits<float>::Max();

		for (int32 k = 0; k < N; k++)
		{
			if (!Matched[k])
			{
				const float d = FVector2D::Distance(A[Idx], B[k]);
				if (d < MinDist)
				{
					MinDist = d;
					ClosestJ = k;
				}
			}
		}

		if (ClosestJ != -1)
		{
			Matched[ClosestJ] = true;
			const float Influence = 1.0f - (float)Count / (float)(N - 1);
			Sum += Influence * MinDist;
		}
	}

	return Sum;
}
