#include "HTTP/GeminiGenerateTextAsync.h"
#include "HTTP/GeminiHTTPManager.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

UGeminiGenerateTextAsync* UGeminiGenerateTextAsync::GenerateText(UObject* WorldContextObject, UAPIData* InAPIData, const FString& UserPrompt, FGeminiGenerateContentConfig InConfig)
{
	UGeminiGenerateTextAsync* Node = NewObject<UGeminiGenerateTextAsync>();
	Node->WorldContextObject = WorldContextObject;
	Node->APIData = InAPIData;
	Node->Prompt = UserPrompt;
	Node->Config = InConfig;
	return Node;
}

void UGeminiGenerateTextAsync::Activate()
{
	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("[GeminiGenerateTextAsync] No WorldContextObject provided"));
		OnCompleted.Broadcast(false, TEXT(""));
		return;
	}
	UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GI)
	{
		UE_LOG(LogTemp, Error, TEXT("[GeminiGenerateTextAsync] Failed to get GameInstance"));
		OnCompleted.Broadcast(false, TEXT(""));
		return;
	}
	UGeminiHTTPManager* Manager = GI->GetSubsystem<UGeminiHTTPManager>();
	if (!Manager)
	{
		UE_LOG(LogTemp, Error, TEXT("[GeminiGenerateTextAsync] Failed to get GeminiHTTPManager subsystem"));
		OnCompleted.Broadcast(false, TEXT(""));
		return;
	}

	Manager->InitializeWithData(APIData);
	FOnGeminiResponse Delegate;
	Delegate.BindUFunction(this, FName("InternalJsonCallback"));
	Manager->GenerateContent(Prompt, Config, Delegate);
}

void UGeminiGenerateTextAsync::InternalJsonCallback(bool bSuccess, const FString& JsonResponse)
{
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GeminiGenerateTextAsync] Request failed"));
		OnCompleted.Broadcast(false, TEXT(""));
		return;
	}
	FString Text;
	if (UGeminiHTTPManager::TryExtractTextFromResponse(JsonResponse, Text))
	{
		UE_LOG(LogTemp, Log, TEXT("[GeminiGenerateTextAsync] Successfully extracted text, length: %d"), Text.Len());
		OnCompleted.Broadcast(true, Text);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[GeminiGenerateTextAsync] Failed to extract text from response"));
		OnCompleted.Broadcast(false, TEXT(""));
	}
}

