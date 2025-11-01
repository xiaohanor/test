// Blueprint Function Library for LLM Action System
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LLM/LLMActionTypes.h"
#include "LLMBlueprintLibrary.generated.h"

class UBlackboardComponent;
class UBehaviorTreeComponent;

/**
 * Helper functions for working with LLM actions in Blueprint
 */
UCLASS()
class TESTCPP_API ULLMBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Complete pipeline: Extract JSON from LLM response, parse, validate, and write to Blackboard
	 * @param LLMResponseBody - Raw JSON response from Gemini API
	 * @param Blackboard - Target blackboard component
	 * @param WorldContext - World context for normalization
	 * @param OutErrorMessage - Error message if pipeline fails
	 * @return true if action was successfully written to blackboard
	 */
	UFUNCTION(BlueprintCallable, Category = "LLM|Actions", meta = (WorldContext = "WorldContext"))
	static bool ProcessLLMResponse(
		const FString& LLMResponseBody,
		UBlackboardComponent* Blackboard,
		UObject* WorldContext,
		FString& OutErrorMessage);

	/**
	 * Get the intent from an FLLMAction as a string
	 */
	UFUNCTION(BlueprintPure, Category = "LLM|Actions")
	static FString GetIntentAsString(const FLLMAction& Action);

	/**
	 * Create a JSON config for structured output with recommended system prompt
	 * Call this to get a properly configured FGeminiGenerateContentConfig for LLM action generation
	 */
	UFUNCTION(BlueprintPure, Category = "LLM|Actions")
	static FString GetLLMActionSystemPrompt();

	/**
	 * Check if an action is valid for execution
	 * @param Action - Action to check
	 * @param OutErrorMessage - Error message if invalid
	 * @return true if action is valid
	 */
	UFUNCTION(BlueprintPure, Category = "LLM|Actions")
	static bool IsActionValid(const FLLMAction& Action, FString& OutErrorMessage);
};
