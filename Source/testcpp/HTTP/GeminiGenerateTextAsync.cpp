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
		OnCompleted.Broadcast(false, TEXT(""));
		return;
	}
	UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GI)
	{
		OnCompleted.Broadcast(false, TEXT(""));
		return;
	}
	UGeminiHTTPManager* Manager = GI->GetSubsystem<UGeminiHTTPManager>();
	if (!Manager)
	{
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
		OnCompleted.Broadcast(false, TEXT(""));
		return;
	}
	FString Text;
	if (UGeminiHTTPManager::TryExtractTextFromResponse(JsonResponse, Text))
	{
		OnCompleted.Broadcast(true, Text);
	}
	else
	{
		OnCompleted.Broadcast(false, TEXT(""));
	}
}

