#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "SplineRendererComponent.generated.h"

UENUM()
enum class ESplineDrawingMode : uint8
{
	DrawAll,
	JustLine,
	JustPoints,
};

UCLASS(hidecategories = (Object, LOD), meta = (BlueprintSpawnableComponent))
class SPLINERENDERERCOMPONENT_API USplineRendererComponent : public USplineComponent
{
	GENERATED_BODY()

public:
	USplineRendererComponent();

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Line Thickness"))
	float LineThickness;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Line Color"))
	FLinearColor LineColor;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Draw Mode"))
	ESplineDrawingMode SplineDrawingMode;

private:
	static void DrawSpline(FPrimitiveDrawInterface* PDI, const FSceneView* View, const FInterpCurveVector& SplineInfo, const FMatrix& LocalToWorld, const FLinearColor& LineColor, uint8 DepthPriorityGroup, const float Thickness, const ESplineDrawingMode SplineDrawingMode);
};


