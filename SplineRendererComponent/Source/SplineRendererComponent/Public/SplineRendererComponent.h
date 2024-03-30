#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "SplineRendererComponent.generated.h"

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

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Draw Points"))
	bool DrawPoints;

private:
	static void DrawSpline(FPrimitiveDrawInterface* PDI, const FSceneView* View, const FInterpCurveVector& SplineInfo, const FMatrix& LocalToWorld, const FLinearColor& LineColor, uint8 DepthPriorityGroup, const float Thickness, const bool DrawPoints);
};


