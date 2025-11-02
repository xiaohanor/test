// Maps parsed LLM actions to Blackboard keys for Behavior Tree execution
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LLM/LLMActionTypes.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "LLMBlackboardMapper.generated.h"

class UBlackboardComponent;

/**
 * Writes validated LLM actions to Blackboard for Behavior Tree consumption
 * 
 * Expected Blackboard keys:
 * - Intent (Name or String): The action intent
 * - TargetLocation (Vector): World position for MoveTo
 * - TargetActor (Object): Actor reference for Interact
 * - TargetId (String): Target identifier
 * - TargetType (String): Target type
 * - SpeakText (String): Text to speak
 * - Confidence (Float): Action confidence
 */
UCLASS(BlueprintType)
class TESTCPP_API ULLMBlackboardMapper : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Write a validated action to the Blackboard
	 * Only writes if action is valid and confidence meets threshold
	 * @param Blackboard - Target blackboard component
	 * @param Action - Action to write
	 * @param ConfidenceThreshold - Minimum confidence to write (default 0.5)
	 * @return true if action was written to blackboard
	 */
	UFUNCTION(BlueprintCallable, Category = "LLM|Blackboard")
	static bool WriteActionToBlackboard(UBlackboardComponent* Blackboard, const FLLMAction& Action, float ConfidenceThreshold = 0.5f);

	/**
	 * Clear all LLM-related keys from the Blackboard
	 * @param Blackboard - Target blackboard component
	 */
	UFUNCTION(BlueprintCallable, Category = "LLM|Blackboard")
	static void ClearLLMKeys(UBlackboardComponent* Blackboard);

	/**
	 * Helper: Get the Blackboard key names that should be set up in BB_LLM asset
	 */
	UFUNCTION(BlueprintPure, Category = "LLM|Blackboard")
	static FString GetRequiredBlackboardKeysDescription();

private:
	// Blackboard key names (must match the keys created in BB_LLM asset)
	static const FName KEY_Intent;
	static const FName KEY_TargetLocation;
	static const FName KEY_TargetActor;
	static const FName KEY_TargetId;
	static const FName KEY_TargetType;
	static const FName KEY_SpeakText;
	static const FName KEY_Confidence;
	static const FName KEY_MontageName;
	static const FName KEY_MontageSection;
	static const FName KEY_MontagePlayRate;
	static const FName KEY_MontageLoop;
};
