// Behavior Tree task to make character speak/display text
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Speak.generated.h"

/**
 * Behavior Tree task that makes the character speak
 * Reads SpeakText from Blackboard and logs it
 * Can be extended to display subtitles, play voice, etc.
 */
UCLASS()
class TESTCPP_API UBTTask_Speak : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Speak();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	// Blackboard key for the text to speak
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector SpeakTextKey;

	// Optional: duration to display the text (for subtitles)
	UPROPERTY(EditAnywhere, Category = "Speech")
	float DisplayDuration = 3.0f;
};
