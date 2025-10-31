// Manages requests to Google Gemini API. Blueprint-friendly wrapper around HTTP/JSON.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "GeminiHTTPManager.generated.h"

class UAPIData;

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnGeminiResponse, bool, bSuccess, const FString&, JsonResponse);

USTRUCT(BlueprintType)
struct FGeminiGenerateContentConfig
{
	GENERATED_BODY()

	// Model name, e.g. "gemini-1.5-flash" or full path "models/gemini-1.5-flash"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gemini")
	FString Model = TEXT("gemini-1.5-flash");

	// System prompt (optional)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gemini")
	FString SystemInstruction;

	// Temperature (0..2 usually)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gemini")
	float Temperature = 0.7f;

	// Max output tokens
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gemini")
	int32 MaxOutputTokens = 2048;

	// Force the model to output JSON only
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gemini|Structured Output")
	bool bForceJsonResponse = false;

	// Optional JSON Schema string to constrain the response shape
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(MultiLine="true"), Category="Gemini|Structured Output")
	FString ResponseSchemaJson;
};

UCLASS(BlueprintType)
class TESTCPP_API UGeminiHTTPManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	// Provide API settings via a Data Asset created in editor (URL must be the base endpoint; APIKey must be valid key)
	UFUNCTION(BlueprintCallable, Category="Gemini")
	void InitializeWithData(UAPIData* InAPIData);

	// Simple text prompt -> JSON string response callback. Non-blocking.
	UFUNCTION(BlueprintCallable, Category="Gemini")
	void GenerateContent(const FString& UserPrompt, const FGeminiGenerateContentConfig& Config, const FOnGeminiResponse& OnDone);

	// Convenience: try to extract concatenated text from Gemini JSON response
	UFUNCTION(BlueprintPure, Category="Gemini")
	static bool TryExtractTextFromResponse(const FString& Json, FString& OutText);

	// Convenience: try to find the first part.text that is valid JSON (object or array) and return it
	UFUNCTION(BlueprintPure, Category="Gemini|Structured Output")
	static bool TryExtractStructuredJsonString(const FString& JsonResponse, FString& OutJsonString);

private:
	// Builds the URL for generateContent endpoint (picks model from APIData->Model if set, otherwise from Config)
	FString BuildGenerateUrl(const FString& Model) const;

	// Builds JSON payload per Google Generative Language API v1
	bool BuildGeneratePayload(const FString& UserPrompt, const FGeminiGenerateContentConfig& Config, FString& OutPayload) const;

	// Handle HTTP response
	void HandleResponse(TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request,
		TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response,
		bool bWasSuccessful,
		FOnGeminiResponse Callback);

private:
	UPROPERTY()
	UAPIData* APIData = nullptr;
};
