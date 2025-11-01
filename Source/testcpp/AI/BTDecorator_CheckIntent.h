// Behavior Tree decorator to check LLM intent
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CheckIntent.generated.h"

/**
 * Decorator that checks if the Intent blackboard key matches expected value
 * Used to branch behavior tree execution based on LLM intent
 */
UCLASS()
class TESTCPP_API UBTDecorator_CheckIntent : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_CheckIntent();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;

protected:
	// Blackboard key for the intent (should be String type)
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IntentKey;

	// Expected intent value to match
	UPROPERTY(EditAnywhere, Category = "Condition")
	FString ExpectedIntent;
};
