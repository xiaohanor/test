// High-level async node for LLM-to-Blackboard pipeline
#include "LLM/LLMGenerateActionAsync.h"
#include "LLM/LLMBlueprintLibrary.h"
#include "HTTP/GeminiHTTPManager.h"
#include "HTTP/APIData.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

ULLMGenerateActionAsync* ULLMGenerateActionAsync::GenerateAction(
	UObject* WorldContextObject,
	UAPIData* InAPIData,
	const FString& InUserInput,
	UBlackboardComponent* InBlackboard,
	float InTemperature)
{
	ULLMGenerateActionAsync* Node = NewObject<ULLMGenerateActionAsync>(GetTransientPackage());
	Node->WorldContextObject = WorldContextObject;
	Node->APIData = InAPIData;
	Node->UserInput = InUserInput;
	Node->Blackboard = InBlackboard;
	Node->Temperature = InTemperature;
	return Node;
}

void ULLMGenerateActionAsync::Activate()
{
	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("[LLMGenerateActionAsync] No WorldContextObject provided"));
		OnCompleted.Broadcast(false, FLLMAction(), TEXT("No WorldContextObject"));
		return;
	}

	if (!Blackboard)
	{
		UE_LOG(LogTemp, Error, TEXT("[LLMGenerateActionAsync] No Blackboard provided"));
		OnCompleted.Broadcast(false, FLLMAction(), TEXT("No Blackboard"));
		return;
	}

	UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GI)
	{
		UE_LOG(LogTemp, Error, TEXT("[LLMGenerateActionAsync] Failed to get GameInstance"));
		OnCompleted.Broadcast(false, FLLMAction(), TEXT("No GameInstance"));
		return;
	}

	UGeminiHTTPManager* Manager = GI->GetSubsystem<UGeminiHTTPManager>();
	if (!Manager)
	{
		UE_LOG(LogTemp, Error, TEXT("[LLMGenerateActionAsync] Failed to get GeminiHTTPManager subsystem"));
		OnCompleted.Broadcast(false, FLLMAction(), TEXT("No GeminiHTTPManager"));
		return;
	}

	// Initialize manager with API data
	Manager->InitializeWithData(APIData);

	// Create config with action system prompt and JSON output
	FGeminiGenerateContentConfig Config;
	Config.Temperature = Temperature;
	Config.SystemInstruction = ULLMBlueprintLibrary::GetLLMActionSystemPrompt();
	Config.bForceJsonResponse = true; // Force JSON-only output

	UE_LOG(LogTemp, Log, TEXT("[LLMGenerateActionAsync] Sending user input to LLM: %s"), *UserInput);

	// Call LLM
	FOnGeminiResponse Delegate;
	Delegate.BindUFunction(this, FName("InternalJsonCallback"));
	Manager->GenerateContent(UserInput, Config, Delegate);
}

void ULLMGenerateActionAsync::InternalJsonCallback(bool bSuccess, const FString& JsonResponse)
{
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("[LLMGenerateActionAsync] LLM request failed"));
		OnCompleted.Broadcast(false, FLLMAction(), TEXT("LLM request failed"));
		return;
	}

	// Process the response using the pipeline
	FString ErrorMessage;
	bool bProcessed = ULLMBlueprintLibrary::ProcessLLMResponse(
		JsonResponse,
		Blackboard,
		WorldContextObject,
		ErrorMessage);

	if (bProcessed)
	{
		// Extract the action from blackboard for the event
		// For now, just create a placeholder action (could read back from blackboard)
		FLLMAction ResultAction;
		UE_LOG(LogTemp, Log, TEXT("[LLMGenerateActionAsync] Successfully generated and processed action"));
		OnCompleted.Broadcast(true, ResultAction, TEXT(""));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[LLMGenerateActionAsync] Failed to process LLM response: %s"), *ErrorMessage);
		OnCompleted.Broadcast(false, FLLMAction(), ErrorMessage);
	}
}
