// Behavior Tree task to play an animation montage on the AI character
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PlayMontage.generated.h"

/**
 * Behavior Tree task that plays an animation montage on the AI character
 * Reads MontageName, MontageSection, MontagePlayRate, and MontageLoop from Blackboard
 * Can play any animation montage configured in the project
 */
UCLASS()
class TESTCPP_API UBTTask_PlayMontage : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_PlayMontage();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	// Blackboard key for the montage name (String)
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector MontageNameKey;

	// Blackboard key for the montage section (String, optional)
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector MontageSectionKey;

	// Blackboard key for the play rate (Float, optional, defaults to 1.0)
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector MontagePlayRateKey;

	// Blackboard key for looping (Bool, optional, defaults to false)
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector MontageLoopKey;

	// Whether to wait for the montage to finish before completing the task
	UPROPERTY(EditAnywhere, Category = "Montage")
	bool bWaitForFinish = false;
};
