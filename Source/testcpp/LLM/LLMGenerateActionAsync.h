// High-level async node for LLM-to-Blackboard pipeline
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HTTP/GeminiHTTPManager.h"
#include "LLM/LLMActionTypes.h"
#include "LLMGenerateActionAsync.generated.h"

class UAPIData;
class UBlackboardComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FLLMActionEvent, bool, bSuccess, const FLLMAction&, Action, const FString&, ErrorMessage);

/**
 * High-level async node that takes user input, calls LLM with action system prompt,
 * parses the response, validates it, and writes to Blackboard
 * 
 * This is a convenience node that wraps the entire M1+M2 pipeline
 */
UCLASS(meta=(BlueprintInternalUseOnly="true"))
class TESTCPP_API ULLMGenerateActionAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * Generate an action from user input via LLM and write to Blackboard
	 * @param WorldContextObject - World context
	 * @param APIData - Gemini API configuration
	 * @param UserInput - Natural language user input
	 * @param Blackboard - Target blackboard to write action to
	 * @param Temperature - LLM temperature (default 0.7)
	 */
	UFUNCTION(BlueprintCallable, Category="LLM|Actions", meta=(BlueprintInternalUseOnly="true", WorldContext="WorldContextObject"))
	static ULLMGenerateActionAsync* GenerateAction(
		UObject* WorldContextObject,
		UAPIData* APIData,
		const FString& UserInput,
		UBlackboardComponent* Blackboard,
		float Temperature = 0.7f);

	virtual void Activate() override;

public:
	// Called when action generation completes (success or failure)
	UPROPERTY(BlueprintAssignable)
	FLLMActionEvent OnCompleted;

private:
	UPROPERTY()
	UObject* WorldContextObject = nullptr;

	UPROPERTY()
	UAPIData* APIData = nullptr;

	UPROPERTY()
	UBlackboardComponent* Blackboard = nullptr;

	FString UserInput;
	float Temperature;

	UFUNCTION()
	void InternalJsonCallback(bool bSuccess, const FString& JsonResponse);
};
