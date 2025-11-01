// Parses and validates LLM JSON output into structured actions
#include "LLM/LLMActionParser.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

bool ULLMActionParser::ParseAction(const FString& JsonText, FLLMAction& OutAction)
{
	OutAction = FLLMAction(); // Reset to defaults
	OutAction.RawJson = JsonText;

	// Parse JSON
	TSharedPtr<FJsonObject> JsonObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
	if (!FJsonSerializer::Deserialize(Reader, JsonObj) || !JsonObj.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[LLMActionParser] Failed to parse JSON: %s"), *JsonText);
		return false;
	}

	// Parse intent (required)
	FString IntentStr;
	if (!JsonObj->TryGetStringField(TEXT("intent"), IntentStr))
	{
		UE_LOG(LogTemp, Error, TEXT("[LLMActionParser] Missing 'intent' field in JSON"));
		return false;
	}
	OutAction.Intent = ParseIntent(IntentStr);

	// Parse target (optional)
	const TSharedPtr<FJsonObject>* TargetObj;
	if (JsonObj->TryGetObjectField(TEXT("target"), TargetObj))
	{
		(*TargetObj)->TryGetStringField(TEXT("id"), OutAction.Target.Id);
		(*TargetObj)->TryGetStringField(TEXT("type"), OutAction.Target.Type);
	}

	// Parse location (optional)
	const TSharedPtr<FJsonObject>* LocationObj;
	if (JsonObj->TryGetObjectField(TEXT("location"), LocationObj))
	{
		// Try to parse as coordinates {x, y, z}
		double X, Y, Z;
		if ((*LocationObj)->TryGetNumberField(TEXT("x"), X) &&
			(*LocationObj)->TryGetNumberField(TEXT("y"), Y) &&
			(*LocationObj)->TryGetNumberField(TEXT("z"), Z))
		{
			OutAction.Location.Coordinates = FVector(X, Y, Z);
			OutAction.Location.bUseCoordinates = true;
		}
	}
	else
	{
		// Try parsing location as a direct string (named point)
		FString LocationStr;
		if (JsonObj->TryGetStringField(TEXT("location"), LocationStr))
		{
			OutAction.Location.NavPointName = LocationStr;
			OutAction.Location.bUseCoordinates = false;
		}
	}

	// Parse speak (optional)
	JsonObj->TryGetStringField(TEXT("speak"), OutAction.Speak);

	// Parse params (optional, keep as JSON string)
	const TSharedPtr<FJsonObject>* ParamsObj;
	if (JsonObj->TryGetObjectField(TEXT("params"), ParamsObj))
	{
		FString ParamsStr;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ParamsStr);
		if (FJsonSerializer::Serialize((*ParamsObj).ToSharedRef(), Writer))
		{
			OutAction.Params = ParamsStr;
		}
	}

	// Parse confidence (optional, default 1.0)
	double Confidence = 1.0;
	if (JsonObj->TryGetNumberField(TEXT("confidence"), Confidence))
	{
		OutAction.Confidence = FMath::Clamp(static_cast<float>(Confidence), 0.0f, 1.0f);
	}
	else
	{
		OutAction.Confidence = 1.0f;
	}

	UE_LOG(LogTemp, Log, TEXT("[LLMActionParser] Parsed action - Intent: %s, Confidence: %.2f"), 
		*IntentToString(OutAction.Intent), OutAction.Confidence);

	return true;
}

bool ULLMActionParser::ValidateAction(const FLLMAction& Action, FString& OutErrorMessage)
{
	OutErrorMessage.Empty();

	// Idle is always valid
	if (Action.Intent == ELLMIntent::Idle)
	{
		return true;
	}

	// Check confidence threshold (hardcoded to 0.5 for MVP)
	if (Action.Confidence < 0.5f)
	{
		OutErrorMessage = FString::Printf(TEXT("Confidence %.2f below threshold 0.5"), Action.Confidence);
		UE_LOG(LogTemp, Warning, TEXT("[LLMActionParser] Validation failed: %s"), *OutErrorMessage);
		return false;
	}

	// Validate MoveTo
	if (Action.Intent == ELLMIntent::MoveTo)
	{
		if (Action.Location.bUseCoordinates)
		{
			if (Action.Location.Coordinates.IsNearlyZero())
			{
				OutErrorMessage = TEXT("MoveTo requires non-zero coordinates");
				UE_LOG(LogTemp, Warning, TEXT("[LLMActionParser] Validation failed: %s"), *OutErrorMessage);
				return false;
			}
		}
		else
		{
			if (Action.Location.NavPointName.IsEmpty())
			{
				OutErrorMessage = TEXT("MoveTo requires either coordinates or NavPointName");
				UE_LOG(LogTemp, Warning, TEXT("[LLMActionParser] Validation failed: %s"), *OutErrorMessage);
				return false;
			}
		}
		return true;
	}

	// Validate Interact
	if (Action.Intent == ELLMIntent::Interact)
	{
		if (Action.Target.Id.IsEmpty() && Action.Target.Type.IsEmpty())
		{
			OutErrorMessage = TEXT("Interact requires target id or type");
			UE_LOG(LogTemp, Warning, TEXT("[LLMActionParser] Validation failed: %s"), *OutErrorMessage);
			return false;
		}
		return true;
	}

	// Validate Speak
	if (Action.Intent == ELLMIntent::Speak)
	{
		if (Action.Speak.IsEmpty())
		{
			OutErrorMessage = TEXT("Speak requires non-empty speak text");
			UE_LOG(LogTemp, Warning, TEXT("[LLMActionParser] Validation failed: %s"), *OutErrorMessage);
			return false;
		}
		// MVP: limit speak length to 500 characters
		if (Action.Speak.Len() > 500)
		{
			OutErrorMessage = TEXT("Speak text exceeds 500 character limit");
			UE_LOG(LogTemp, Warning, TEXT("[LLMActionParser] Validation failed: %s"), *OutErrorMessage);
			return false;
		}
		return true;
	}

	// Unknown intent
	OutErrorMessage = FString::Printf(TEXT("Unknown intent: %s"), *IntentToString(Action.Intent));
	UE_LOG(LogTemp, Warning, TEXT("[LLMActionParser] Validation failed: %s"), *OutErrorMessage);
	return false;
}

bool ULLMActionParser::NormalizeAction(FLLMAction& Action, UObject* WorldContext)
{
	// If using named navigation point, try to resolve it to coordinates
	if (Action.Intent == ELLMIntent::MoveTo && !Action.Location.bUseCoordinates)
	{
		// For MVP, we'll just log that named points need to be resolved by game-specific logic
		// In a real implementation, you'd query a navigation point registry here
		UE_LOG(LogTemp, Warning, TEXT("[LLMActionParser] Named navigation point '%s' requires game-specific resolution"), 
			*Action.Location.NavPointName);
		// For now, leave it as-is and let the BT handle it
	}

	// Apply any default values
	if (Action.Confidence == 0.0f)
	{
		Action.Confidence = 1.0f;
	}

	return true;
}

FString ULLMActionParser::GetRecommendedSystemPrompt()
{
	return TEXT(
		"You are an AI assistant that converts natural language commands into structured JSON actions for a game character. "
		"You must ONLY output valid JSON with no additional text or explanation.\n\n"
		"Supported intents:\n"
		"- MoveTo: Move character to a location\n"
		"- Interact: Interact with an object\n"
		"- Speak: Make character speak\n\n"
		"JSON schema:\n"
		"{\n"
		"  \"intent\": \"MoveTo\" | \"Interact\" | \"Speak\",\n"
		"  \"target\": {\"id\": \"string\", \"type\": \"string\"} (optional, for Interact),\n"
		"  \"location\": {\"x\": 0, \"y\": 0, \"z\": 0} | \"NavPointName\" (for MoveTo),\n"
		"  \"speak\": \"text to say\" (for Speak),\n"
		"  \"params\": {} (optional),\n"
		"  \"confidence\": 0.0-1.0 (required)\n"
		"}\n\n"
		"Examples:\n"
		"User: \"Go to the fountain\"\n"
		"{\"intent\":\"MoveTo\",\"location\":\"Fountain\",\"confidence\":0.9}\n\n"
		"User: \"Talk to the guard\"\n"
		"{\"intent\":\"Interact\",\"target\":{\"type\":\"Guard\"},\"confidence\":0.85}\n\n"
		"User: \"Say hello\"\n"
		"{\"intent\":\"Speak\",\"speak\":\"Hello\",\"confidence\":0.95}\n\n"
		"Output ONLY valid JSON. No markdown, no explanation."
	);
}

ELLMIntent ULLMActionParser::ParseIntent(const FString& IntentStr)
{
	if (IntentStr.Equals(TEXT("MoveTo"), ESearchCase::IgnoreCase))
	{
		return ELLMIntent::MoveTo;
	}
	if (IntentStr.Equals(TEXT("Interact"), ESearchCase::IgnoreCase))
	{
		return ELLMIntent::Interact;
	}
	if (IntentStr.Equals(TEXT("Speak"), ESearchCase::IgnoreCase))
	{
		return ELLMIntent::Speak;
	}
	if (IntentStr.Equals(TEXT("Idle"), ESearchCase::IgnoreCase))
	{
		return ELLMIntent::Idle;
	}

	UE_LOG(LogTemp, Warning, TEXT("[LLMActionParser] Unknown intent '%s', defaulting to Idle"), *IntentStr);
	return ELLMIntent::Idle;
}

FString ULLMActionParser::IntentToString(ELLMIntent Intent)
{
	switch (Intent)
	{
	case ELLMIntent::MoveTo: return TEXT("MoveTo");
	case ELLMIntent::Interact: return TEXT("Interact");
	case ELLMIntent::Speak: return TEXT("Speak");
	case ELLMIntent::Idle: return TEXT("Idle");
	default: return TEXT("Unknown");
	}
}
