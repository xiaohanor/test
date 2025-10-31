#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HTTP/GeminiHTTPManager.h"
#include "GeminiGenerateTextAsync.generated.h"

class UAPIData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGeminiTextEvent, bool, bSuccess, const FString&, Text);

UCLASS(meta=(BlueprintInternalUseOnly="true"))
class TESTCPP_API UGeminiGenerateTextAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	// Async node entry for Blueprints: returns plain text
	UFUNCTION(BlueprintCallable, Category="Gemini", meta=(BlueprintInternalUseOnly="true", WorldContext="WorldContextObject"))
	static UGeminiGenerateTextAsync* GenerateText(UObject* WorldContextObject, UAPIData* APIData, const FString& UserPrompt, FGeminiGenerateContentConfig Config);

	virtual void Activate() override;

public:
	UPROPERTY(BlueprintAssignable)
	FGeminiTextEvent OnCompleted;

private:
	UPROPERTY()
	UObject* WorldContextObject = nullptr;

	UPROPERTY()
	UAPIData* APIData = nullptr;

	FString Prompt;
	FGeminiGenerateContentConfig Config;

	UFUNCTION()
	void InternalJsonCallback(bool bSuccess, const FString& JsonResponse);
};

