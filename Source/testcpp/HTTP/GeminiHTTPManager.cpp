#include "HTTP/GeminiHTTPManager.h"
#include "HTTP/APIData.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "GenericPlatform/GenericPlatformHttp.h"

void UGeminiHTTPManager::InitializeWithData(UAPIData* InAPIData)
{
	APIData = InAPIData;
}

void UGeminiHTTPManager::GenerateContent(const FString& UserPrompt, const FGeminiGenerateContentConfig& Config, const FOnGeminiResponse& OnDone)
{
	if (!APIData)
	{
		UE_LOG(LogTemp, Error, TEXT("[GeminiHTTP] Not initialized: APIData is null"));
		OnDone.ExecuteIfBound(false, TEXT("{""error"": ""No APIData""}"));
		return;
	}

	// Prefer model from APIData, then Config, then default
	FString EffectiveModel = APIData->GetModel();
	if (EffectiveModel.IsEmpty())
	{
		EffectiveModel = Config.Model.IsEmpty() ? TEXT("gemini-1.5-flash") : Config.Model;
	}

	FString Url = BuildGenerateUrl(EffectiveModel);
	FString Payload;
	if (!BuildGeneratePayload(UserPrompt, Config, Payload))
	{
		UE_LOG(LogTemp, Error, TEXT("[GeminiHTTP] Failed to build request payload"));
		OnDone.ExecuteIfBound(false, TEXT("{""error"": ""Failed to build payload""}"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[GeminiHTTP] Request URL: %s"), *Url);
	UE_LOG(LogTemp, Log, TEXT("[GeminiHTTP] Request Payload: %s"), *Payload);

	FHttpModule& Http = FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http.CreateRequest();

	// Some Gemini endpoints accept API key via query string: ?key=API_KEY
	if (!APIData->GetAPIKey().IsEmpty())
	{
		const TCHAR* Delim = Url.Contains(TEXT("?")) ? TEXT("&") : TEXT("?");
		const FString EncKey = FGenericPlatformHttp::UrlEncode(APIData->GetAPIKey());
		Url += FString::Printf(TEXT("%skey=%s"), Delim, *EncKey);
	}

	Request->SetURL(Url);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(Payload);

	Request->OnProcessRequestComplete().BindUObject(this, &UGeminiHTTPManager::HandleResponse, OnDone);
	Request->ProcessRequest();
}

bool UGeminiHTTPManager::TryExtractTextFromResponse(const FString& Json, FString& OutText)
{
	OutText.Empty();
	TSharedPtr<FJsonObject> RootObj;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	if (!FJsonSerializer::Deserialize(Reader, RootObj) || !RootObj.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GeminiHTTP] TryExtractText: Failed to parse JSON"));
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>* CandidatesArr = nullptr;
	if (!RootObj->TryGetArrayField(TEXT("candidates"), CandidatesArr) || CandidatesArr == nullptr || CandidatesArr->Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GeminiHTTP] TryExtractText: No candidates found"));
		return false;
	}

	// Typically take first candidate
	const TSharedPtr<FJsonObject>* CandidateObjPtr = nullptr;
	if (!(*CandidatesArr)[0]->TryGetObject(CandidateObjPtr) || CandidateObjPtr == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GeminiHTTP] TryExtractText: First candidate invalid"));
		return false;
	}
	const FJsonObject& CandidateObj = *CandidateObjPtr->Get();

	const TSharedPtr<FJsonObject>* ContentObjPtr = nullptr;
	if (!CandidateObj.TryGetObjectField(TEXT("content"), ContentObjPtr) || ContentObjPtr == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GeminiHTTP] TryExtractText: No content field"));
		return false;
	}
	const FJsonObject& ContentObj = *ContentObjPtr->Get();

	const TArray<TSharedPtr<FJsonValue>>* PartsArr = nullptr;
	if (!ContentObj.TryGetArrayField(TEXT("parts"), PartsArr) || PartsArr == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GeminiHTTP] TryExtractText: No parts array"));
		return false;
	}

	FString Accum;
	for (const TSharedPtr<FJsonValue>& PartVal : *PartsArr)
	{
		if (!PartVal.IsValid()) continue;
		const TSharedPtr<FJsonObject>* PartObjPtr = nullptr;
		if (PartVal->TryGetObject(PartObjPtr) && PartObjPtr && PartObjPtr->IsValid())
		{
			FString Text;
			if ((*PartObjPtr)->TryGetStringField(TEXT("text"), Text))
			{
				Accum += Text;
			}
		}
	}

	OutText = Accum;
	if (!OutText.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("[GeminiHTTP] Extracted text: %s"), *OutText);
	}
	return !OutText.IsEmpty();
}

bool UGeminiHTTPManager::TryExtractStructuredJsonString(const FString& JsonResponse, FString& OutJsonString)
{
	OutJsonString.Empty();
	FString Text;
	if (!TryExtractTextFromResponse(JsonResponse, Text))
	{
		return false;
	}
	// Heuristic: extract the first top-level JSON object/array substring
	int32 Start = INDEX_NONE;
	int32 End = INDEX_NONE;
	for (int32 i = 0; i < Text.Len(); ++i)
	{
		TCHAR C = Text[i];
		if (C == TEXT('{') || C == TEXT('[')) { Start = i; break; }
	}
	if (Start == INDEX_NONE) return false;

	// Find matching closing brace/bracket using a simple stack counter
	TCHAR Open = Text[Start];
	TCHAR Close = (Open == TEXT('{')) ? TEXT('}') : TEXT(']');
	int32 Depth = 0;
	for (int32 i = Start; i < Text.Len(); ++i)
	{
		TCHAR C = Text[i];
		if (C == Open) Depth++;
		else if (C == Close) {
			Depth--;
			if (Depth == 0) { End = i; break; }
		}
	}
	if (End == INDEX_NONE) return false;

	OutJsonString = Text.Mid(Start, End - Start + 1);
	return true;
}

FString UGeminiHTTPManager::BuildGenerateUrl(const FString& Model) const
{
	// Expected base URL example: https://generativelanguage.googleapis.com/v1
	FString Base = APIData ? APIData->GetURL() : TEXT("https://generativelanguage.googleapis.com/v1");
	Base.RemoveFromEnd(TEXT("/"));
	FString ModelPath = Model;
	if (ModelPath.IsEmpty())
	{
		ModelPath = TEXT("models/gemini-1.5-flash");
	}
	if (!ModelPath.StartsWith(TEXT("models/")))
	{
		ModelPath = FString::Printf(TEXT("models/%s"), *ModelPath);
	}
	return FString::Printf(TEXT("%s/%s:generateContent"), *Base, *ModelPath);
}

bool UGeminiHTTPManager::BuildGeneratePayload(const FString& UserPrompt, const FGeminiGenerateContentConfig& Config, FString& OutPayload) const
{
	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();

	// contents: [{ role: "user", parts: [{ text: "..." }] }]
	TArray<TSharedPtr<FJsonValue>> Contents;
	{
		TSharedRef<FJsonObject> ContentObj = MakeShared<FJsonObject>();
		ContentObj->SetStringField(TEXT("role"), TEXT("user"));

		TArray<TSharedPtr<FJsonValue>> Parts;
		{
			TSharedRef<FJsonObject> PartObj = MakeShared<FJsonObject>();
			PartObj->SetStringField(TEXT("text"), UserPrompt);
			Parts.Add(MakeShared<FJsonValueObject>(PartObj));
		}
		ContentObj->SetArrayField(TEXT("parts"), Parts);
		Contents.Add(MakeShared<FJsonValueObject>(ContentObj));
	}
	Root->SetArrayField(TEXT("contents"), Contents);

	// generationConfig
	{
		TSharedRef<FJsonObject> GenCfg = MakeShared<FJsonObject>();
		GenCfg->SetNumberField(TEXT("temperature"), Config.Temperature);
		GenCfg->SetNumberField(TEXT("maxOutputTokens"), Config.MaxOutputTokens);

		if (Config.bForceJsonResponse)
		{
			GenCfg->SetStringField(TEXT("response_mime_type"), TEXT("application/json"));
		}

		Root->SetObjectField(TEXT("generationConfig"), GenCfg);
	}

	// systemInstruction (optional)
	if (!Config.SystemInstruction.IsEmpty())
	{
		TSharedRef<FJsonObject> SysContent = MakeShared<FJsonObject>();
		TArray<TSharedPtr<FJsonValue>> SysParts;
		{
			TSharedRef<FJsonObject> PartObj = MakeShared<FJsonObject>();
			PartObj->SetStringField(TEXT("text"), Config.SystemInstruction);
			SysParts.Add(MakeShared<FJsonValueObject>(PartObj));
		}
		SysContent->SetArrayField(TEXT("parts"), SysParts);
		Root->SetObjectField(TEXT("systemInstruction"), SysContent);
	}

	// response_schema (optional)
	if (!Config.ResponseSchemaJson.IsEmpty())
	{
		TSharedPtr<FJsonObject> SchemaObj;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Config.ResponseSchemaJson);
		if (FJsonSerializer::Deserialize(Reader, SchemaObj) && SchemaObj.IsValid())
		{
			Root->SetObjectField(TEXT("response_schema"), SchemaObj.ToSharedRef());
		}
	}

	// Serialize
	TSharedPtr<FJsonObject> RootPtr = Root;
	if (!RootPtr.IsValid()) return false;

	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutPayload);
	return FJsonSerializer::Serialize(RootPtr.ToSharedRef(), Writer);
}

void UGeminiHTTPManager::HandleResponse(TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request,
	TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response,
	bool bWasSuccessful,
	FOnGeminiResponse Callback)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[GeminiHTTP] Request failed or no response received"));
		Callback.ExecuteIfBound(false, TEXT("{""error"": ""No response""}"));
		return;
	}

	const int32 Code = Response->GetResponseCode();
	const FString Body = Response->GetContentAsString();
	const bool bOk = Code >= 200 && Code < 300;
	
	if (bOk)
	{
		UE_LOG(LogTemp, Log, TEXT("[GeminiHTTP] Response (Code %d): %s"), Code, *Body);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[GeminiHTTP] Error Response (Code %d): %s"), Code, *Body);
	}
	
	Callback.ExecuteIfBound(bOk, Body);
}
