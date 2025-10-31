#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HTTP/GeminiHTTPManager.h"
#include "GeminiGenerateContentAsync.generated.h"

class UAPIData;
class UGeminiHTTPManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGeminiResponseEvent, bool, bSuccess, const FString&, JsonResponse);

UCLASS(meta=(BlueprintInternalUseOnly="true"))
class TESTCPP_API UGeminiGenerateContentAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	// Entry point callable from Blueprints
	UFUNCTION(BlueprintCallable, Category="Gemini", meta=(BlueprintInternalUseOnly="true", WorldContext="WorldContextObject"))
	static UGeminiGenerateContentAsync* GenerateContent(UObject* WorldContextObject, UAPIData* APIData, const FString& UserPrompt, FGeminiGenerateContentConfig Config);

	// Begin UBlueprintAsyncActionBase
	virtual void Activate() override;
	// End UBlueprintAsyncActionBase

public:
	UPROPERTY(BlueprintAssignable)
	FGeminiResponseEvent OnCompleted;

private:
	UPROPERTY()
	UObject* WorldContextObject = nullptr;

	UPROPERTY()
	UAPIData* APIData = nullptr;

	FString Prompt;
	FGeminiGenerateContentConfig Config;

	UFUNCTION()
	void InternalCallback(bool bSuccess, const FString& Json);
};
