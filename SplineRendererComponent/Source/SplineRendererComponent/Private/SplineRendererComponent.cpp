#include "SplineRendererComponent.h"

USplineRendererComponent::USplineRendererComponent()
	: LineThickness(1.f)
	, LineColor(FLinearColor::White)
	, SplineDrawingMode(ESplineDrawingMode::DrawAll)
{}

FPrimitiveSceneProxy* USplineRendererComponent::CreateSceneProxy()
{
	class FSplineSceneProxy final : public FPrimitiveSceneProxy
	{
	public:
		SIZE_T GetTypeHash() const override
		{
			static size_t UniquePointer;
			return reinterpret_cast<size_t>(&UniquePointer);
		}

		FSplineSceneProxy(const USplineRendererComponent* InComponent)
			: FPrimitiveSceneProxy(InComponent)			
			, SplineInfo(InComponent->SplineCurves.Position)
			, LineColor(InComponent->LineColor)
			, Thickness(InComponent->LineThickness)
			, SplineDrawingMode(InComponent->SplineDrawingMode)
		{}

		virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
		{
			QUICK_SCOPE_CYCLE_COUNTER(STAT_SplineSceneProxy_GetDynamicMeshElements);

			if (IsSelected())
			{
				return;
			}

			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (VisibilityMap & (1 << ViewIndex))
				{
					const FSceneView* View = Views[ViewIndex];
					FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);

					const FMatrix& LocalToWorld = GetLocalToWorld();

					// Taking into account the min and maximum drawing distance
					const float DistanceSqr = (View->ViewMatrices.GetViewOrigin() - LocalToWorld.GetOrigin()).SizeSquared();
					if (DistanceSqr < FMath::Square(GetMinDrawDistance()) || DistanceSqr > FMath::Square(GetMaxDrawDistance()))
					{
						continue;
					}

					USplineRendererComponent::DrawSpline(PDI, View, SplineInfo, LocalToWorld, LineColor, SDPG_World, Thickness, SplineDrawingMode);
				}
			}
		}

		virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
		{
			FPrimitiveViewRelevance Result;
			Result.bDrawRelevance = true;
			Result.bDynamicRelevance = IsShown(View);
			Result.bShadowRelevance = IsShadowCast(View);
			Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
			return Result;
		}

		virtual uint32 GetMemoryFootprint(void) const override { return sizeof * this + GetAllocatedSize(); }
		uint32 GetAllocatedSize(void) const { return FPrimitiveSceneProxy::GetAllocatedSize(); }

	private:
		FInterpCurveVector SplineInfo;
		FLinearColor LineColor;
		float Thickness;
		ESplineDrawingMode SplineDrawingMode;
	};

	return new FSplineSceneProxy(this);
}

void USplineRendererComponent::DrawSpline(FPrimitiveDrawInterface* PDI, const FSceneView* View, const FInterpCurveVector& SplineInfo, const FMatrix& LocalToWorld, const FLinearColor& LineColor, uint8 DepthPriorityGroup, const float Thickness, const ESplineDrawingMode SplineDrawingMode)
{
	const int32 GrabHandleSize = 6;
	FVector OldKeyPos(0);

	const int32 NumPoints = SplineInfo.Points.Num();
	const int32 NumSegments = SplineInfo.bIsLooped ? NumPoints : NumPoints - 1;
	for (int32 KeyIdx = 0; KeyIdx < NumSegments + 1; KeyIdx++)
	{
		const FVector NewKeyPos = LocalToWorld.TransformPosition(SplineInfo.Eval(static_cast<float>(KeyIdx), FVector(0)));

		if (SplineDrawingMode == ESplineDrawingMode::JustPoints || SplineDrawingMode == ESplineDrawingMode::DrawAll)
		{
			// Draw the keypoint
			if (KeyIdx < NumPoints)
			{
				PDI->DrawPoint(NewKeyPos, LineColor, GrabHandleSize, DepthPriorityGroup);
			}
		}		

		// If not the first keypoint, draw a line to the previous keypoint.
		if (KeyIdx > 0 && (SplineDrawingMode == ESplineDrawingMode::JustLine || SplineDrawingMode == ESplineDrawingMode::DrawAll))
		{
			// For constant interpolation - don't draw ticks - just draw dotted line.
			if (SplineInfo.Points[KeyIdx - 1].InterpMode == CIM_Constant)
			{
				// Calculate dash length according to size on screen
				const float StartW = View->WorldToScreen(OldKeyPos).W;
				const float EndW = View->WorldToScreen(NewKeyPos).W;

				const float WLimit = 10.0f;
				if (StartW > WLimit || EndW > WLimit)
				{
					const float Scale = 0.03f;
					DrawDashedLine(PDI, OldKeyPos, NewKeyPos, LineColor, FMath::Max(StartW, EndW) * Scale, DepthPriorityGroup);
				}
			}
			else
			{
				// Find position on first keyframe.
				FVector OldPos = OldKeyPos;

				// Then draw a line for each substep.
				const int32 NumSteps = 20;

				for (int32 StepIdx = 1; StepIdx <= NumSteps; StepIdx++)
				{
					const float Key = (KeyIdx - 1) + (StepIdx / static_cast<float>(NumSteps));
					const FVector NewPos = LocalToWorld.TransformPosition(SplineInfo.Eval(Key, FVector(0)));
					PDI->DrawLine(OldPos, NewPos, LineColor, DepthPriorityGroup, Thickness);
					OldPos = NewPos;
				}
			}
		}

		OldKeyPos = NewKeyPos;
	}
}