// Behavior Tree task to interact with a target actor
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_InteractTarget.generated.h"

/**
 * Behavior Tree task that interacts with the target actor
 * Requires TargetActor to be set in Blackboard
 * For MVP, logs the interaction; can be extended to call IInteractable interface
 */
UCLASS()
class TESTCPP_API UBTTask_InteractTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_InteractTarget();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	// Blackboard key for the target actor
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	// Blackboard key for target ID (fallback if actor not set)
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetIdKey;

	// Blackboard key for target type (additional context)
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetTypeKey;
};
