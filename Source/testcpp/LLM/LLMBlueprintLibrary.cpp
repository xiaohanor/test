// Blueprint Function Library for LLM Action System
#include "LLM/LLMBlueprintLibrary.h"
#include "LLM/LLMActionParser.h"
#include "LLM/LLMBlackboardMapper.h"
#include "HTTP/GeminiHTTPManager.h"
#include "BehaviorTree/BlackboardComponent.h"

// Use default confidence threshold
static constexpr float DefaultConfidenceThreshold = 0.5f;

bool ULLMBlueprintLibrary::ProcessLLMResponse(
	const FString& LLMResponseBody,
	UBlackboardComponent* Blackboard,
	UObject* WorldContext,
	FString& OutErrorMessage)
{
	OutErrorMessage.Empty();

	if (!Blackboard)
	{
		OutErrorMessage = TEXT("Blackboard is null");
		UE_LOG(LogTemp, Error, TEXT("[LLMBlueprintLibrary] %s"), *OutErrorMessage);
		return false;
	}

	// Step 1: Extract JSON from LLM response
	FString JsonString;
	if (!UGeminiHTTPManager::TryExtractStructuredJsonString(LLMResponseBody, JsonString))
	{
		OutErrorMessage = TEXT("Failed to extract JSON from LLM response");
		UE_LOG(LogTemp, Error, TEXT("[LLMBlueprintLibrary] %s"), *OutErrorMessage);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[LLMBlueprintLibrary] Extracted JSON: %s"), *JsonString);

	// Step 2: Parse JSON to action
	FLLMAction Action;
	if (!ULLMActionParser::ParseAction(JsonString, Action))
	{
		OutErrorMessage = TEXT("Failed to parse JSON to action");
		UE_LOG(LogTemp, Error, TEXT("[LLMBlueprintLibrary] %s"), *OutErrorMessage);
		return false;
	}

	// Step 3: Validate action
	if (!ULLMActionParser::ValidateAction(Action, OutErrorMessage))
	{
		UE_LOG(LogTemp, Warning, TEXT("[LLMBlueprintLibrary] Action validation failed: %s"), *OutErrorMessage);
		return false;
	}

	// Step 4: Normalize action
	if (!ULLMActionParser::NormalizeAction(Action, WorldContext))
	{
		OutErrorMessage = TEXT("Failed to normalize action");
		UE_LOG(LogTemp, Warning, TEXT("[LLMBlueprintLibrary] %s"), *OutErrorMessage);
		// Continue anyway, normalization is not critical
	}

	// Step 5: Write to blackboard (using default threshold)
	if (!ULLMBlackboardMapper::WriteActionToBlackboard(Blackboard, Action, DefaultConfidenceThreshold))
	{
		OutErrorMessage = TEXT("Failed to write action to blackboard");
		UE_LOG(LogTemp, Error, TEXT("[LLMBlueprintLibrary] %s"), *OutErrorMessage);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[LLMBlueprintLibrary] Successfully processed LLM response and updated blackboard"));
	return true;
}

FString ULLMBlueprintLibrary::GetIntentAsString(const FLLMAction& Action)
{
	switch (Action.Intent)
	{
	case ELLMIntent::MoveTo: return TEXT("MoveTo");
	case ELLMIntent::Interact: return TEXT("Interact");
	case ELLMIntent::Speak: return TEXT("Speak");
	case ELLMIntent::PlayMontage: return TEXT("PlayMontage");
	case ELLMIntent::Idle: return TEXT("Idle");
	default: return TEXT("Unknown");
	}
}

FString ULLMBlueprintLibrary::GetLLMActionSystemPrompt()
{
	return ULLMActionParser::GetRecommendedSystemPrompt();
}

bool ULLMBlueprintLibrary::IsActionValid(const FLLMAction& Action, FString& OutErrorMessage)
{
	return ULLMActionParser::ValidateAction(Action, OutErrorMessage);
}
