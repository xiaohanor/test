// Parses and validates LLM JSON output into structured actions
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LLM/LLMActionTypes.h"
#include "LLMActionParser.generated.h"

/**
 * Parses, validates, and normalizes LLM JSON output
 * Converts structured JSON from LLM into FLLMAction structs
 */
UCLASS(BlueprintType)
class TESTCPP_API ULLMActionParser : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Parse JSON string from LLM into an FLLMAction struct
	 * @param JsonText - JSON string to parse (should be structured JSON from LLM)
	 * @param OutAction - Populated action struct
	 * @return true if parsing succeeded, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "LLM|Parser")
	static bool ParseAction(const FString& JsonText, FLLMAction& OutAction);

	/**
	 * Validate that an action has required fields and correct types
	 * @param Action - Action to validate
	 * @param OutErrorMessage - Error message if validation fails
	 * @return true if action is valid
	 */
	UFUNCTION(BlueprintCallable, Category = "LLM|Parser")
	static bool ValidateAction(const FLLMAction& Action, FString& OutErrorMessage);

	/**
	 * Normalize action: apply defaults, resolve named locations to coordinates
	 * @param Action - Action to normalize (modified in place)
	 * @param WorldContext - World context for navigation point lookup
	 * @return true if normalization succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "LLM|Parser", meta = (WorldContext = "WorldContext"))
	static bool NormalizeAction(UPARAM(ref) FLLMAction& Action, UObject* WorldContext);

	/**
	 * Recommended system prompt for LLM to produce valid JSON actions
	 * Use this in your Config.SystemInstruction when calling GenerateContent
	 */
	UFUNCTION(BlueprintPure, Category = "LLM|Parser")
	static FString GetRecommendedSystemPrompt();

private:
	// Helper: parse intent string to enum
	static ELLMIntent ParseIntent(const FString& IntentStr);

	// Helper: intent enum to string
	static FString IntentToString(ELLMIntent Intent);
};
