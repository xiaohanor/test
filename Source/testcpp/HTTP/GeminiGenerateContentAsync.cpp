#include "HTTP/GeminiGenerateContentAsync.h"
#include "HTTP/GeminiHTTPManager.h"
#include "HTTP/APIData.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

UGeminiGenerateContentAsync* UGeminiGenerateContentAsync::GenerateContent(UObject* WorldContextObject, UAPIData* InAPIData, const FString& UserPrompt, FGeminiGenerateContentConfig InConfig)
{
	UGeminiGenerateContentAsync* Node = NewObject<UGeminiGenerateContentAsync>();
	Node->WorldContextObject = WorldContextObject;
	Node->APIData = InAPIData;
	Node->Prompt = UserPrompt;
	Node->Config = InConfig;
	return Node;
}

void UGeminiGenerateContentAsync::Activate()
{
	if (!WorldContextObject)
	{
		OnCompleted.Broadcast(false, TEXT("{""error"": ""No world context""}"));
		return;
	}
	UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GI)
	{
		OnCompleted.Broadcast(false, TEXT("{""error"": ""No GameInstance""}"));
		return;
	}

	UGeminiHTTPManager* Manager = GI->GetSubsystem<UGeminiHTTPManager>();
	if (!Manager)
	{
		OnCompleted.Broadcast(false, TEXT("{""error"": ""GeminiHTTPManager not available""}"));
		return;
	}

	Manager->InitializeWithData(APIData);
	FGeminiGenerateContentConfig LocalConfig = Config; // copy
	FOnGeminiResponse Delegate;
	Delegate.BindUFunction(this, FName("InternalCallback"));
	Manager->GenerateContent(Prompt, LocalConfig, Delegate);
}

void UGeminiGenerateContentAsync::InternalCallback(bool bSuccess, const FString& Json)
{
	OnCompleted.Broadcast(bSuccess, Json);
}

