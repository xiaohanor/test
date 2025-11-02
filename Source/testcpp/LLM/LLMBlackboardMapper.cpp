// Maps parsed LLM actions to Blackboard keys for Behavior Tree execution
#include "LLM/LLMBlackboardMapper.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "LLM/LLMActionParser.h"

// Define Blackboard key names
const FName ULLMBlackboardMapper::KEY_Intent = FName(TEXT("Intent"));
const FName ULLMBlackboardMapper::KEY_TargetLocation = FName(TEXT("TargetLocation"));
const FName ULLMBlackboardMapper::KEY_TargetActor = FName(TEXT("TargetActor"));
const FName ULLMBlackboardMapper::KEY_TargetId = FName(TEXT("TargetId"));
const FName ULLMBlackboardMapper::KEY_TargetType = FName(TEXT("TargetType"));
const FName ULLMBlackboardMapper::KEY_SpeakText = FName(TEXT("SpeakText"));
const FName ULLMBlackboardMapper::KEY_Confidence = FName(TEXT("Confidence"));
const FName ULLMBlackboardMapper::KEY_MontageName = FName(TEXT("MontageName"));
const FName ULLMBlackboardMapper::KEY_MontageSection = FName(TEXT("MontageSection"));
const FName ULLMBlackboardMapper::KEY_MontagePlayRate = FName(TEXT("MontagePlayRate"));
const FName ULLMBlackboardMapper::KEY_MontageLoop = FName(TEXT("MontageLoop"));

bool ULLMBlackboardMapper::WriteActionToBlackboard(UBlackboardComponent* Blackboard, const FLLMAction& Action, float ConfidenceThreshold)
{
	if (!Blackboard)
	{
		UE_LOG(LogTemp, Error, TEXT("[LLMBlackboardMapper] Blackboard is null"));
		return false;
	}

	// Check confidence threshold
	if (Action.Confidence < ConfidenceThreshold)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LLMBlackboardMapper] Action confidence %.2f below threshold %.2f, not writing to blackboard"),
			Action.Confidence, ConfidenceThreshold);
		return false;
	}

	// Validate action before writing
	FString ValidationError;
	if (!ULLMActionParser::ValidateAction(Action, ValidationError))
	{
		UE_LOG(LogTemp, Warning, TEXT("[LLMBlackboardMapper] Action validation failed: %s"), *ValidationError);
		return false;
	}

	// Clear previous values
	ClearLLMKeys(Blackboard);

	// Write Intent as String (convert enum to string)
	FString IntentStr;
	switch (Action.Intent)
	{
	case ELLMIntent::MoveTo: IntentStr = TEXT("MoveTo"); break;
	case ELLMIntent::Interact: IntentStr = TEXT("Interact"); break;
	case ELLMIntent::Speak: IntentStr = TEXT("Speak"); break;
	case ELLMIntent::PlayMontage: IntentStr = TEXT("PlayMontage"); break;
	case ELLMIntent::Idle: IntentStr = TEXT("Idle"); break;
	default: IntentStr = TEXT("Idle"); break;
	}
	Blackboard->SetValueAsString(KEY_Intent, IntentStr);
	UE_LOG(LogTemp, Log, TEXT("[LLMBlackboardMapper] Set Intent: %s"), *IntentStr);

	// Write Confidence
	Blackboard->SetValueAsFloat(KEY_Confidence, Action.Confidence);

	// Write intent-specific data
	if (Action.Intent == ELLMIntent::MoveTo)
	{
		if (Action.Location.bUseCoordinates)
		{
			Blackboard->SetValueAsVector(KEY_TargetLocation, Action.Location.Coordinates);
			UE_LOG(LogTemp, Log, TEXT("[LLMBlackboardMapper] Set TargetLocation: %s"), *Action.Location.Coordinates.ToString());
		}
		else
		{
			// If using named point, store the name in TargetId for resolution later
			Blackboard->SetValueAsString(KEY_TargetId, Action.Location.NavPointName);
			UE_LOG(LogTemp, Log, TEXT("[LLMBlackboardMapper] Set TargetId (NavPoint): %s"), *Action.Location.NavPointName);
		}
	}
	else if (Action.Intent == ELLMIntent::Interact)
	{
		if (!Action.Target.Id.IsEmpty())
		{
			Blackboard->SetValueAsString(KEY_TargetId, Action.Target.Id);
			UE_LOG(LogTemp, Log, TEXT("[LLMBlackboardMapper] Set TargetId: %s"), *Action.Target.Id);
		}
		if (!Action.Target.Type.IsEmpty())
		{
			Blackboard->SetValueAsString(KEY_TargetType, Action.Target.Type);
			UE_LOG(LogTemp, Log, TEXT("[LLMBlackboardMapper] Set TargetType: %s"), *Action.Target.Type);
		}
	}
	else if (Action.Intent == ELLMIntent::Speak)
	{
		Blackboard->SetValueAsString(KEY_SpeakText, Action.Speak);
		UE_LOG(LogTemp, Log, TEXT("[LLMBlackboardMapper] Set SpeakText: %s"), *Action.Speak);
	}
	else if (Action.Intent == ELLMIntent::PlayMontage)
	{
		Blackboard->SetValueAsString(KEY_MontageName, Action.Montage.Name);
		Blackboard->SetValueAsString(KEY_MontageSection, Action.Montage.Section);
		Blackboard->SetValueAsFloat(KEY_MontagePlayRate, Action.Montage.PlayRate);
		Blackboard->SetValueAsBool(KEY_MontageLoop, Action.Montage.bLoop);
		UE_LOG(LogTemp, Log, TEXT("[LLMBlackboardMapper] Set Montage: Name=%s, Section=%s, Rate=%.2f, Loop=%s"),
			*Action.Montage.Name,
			Action.Montage.Section.IsEmpty() ? TEXT("None") : *Action.Montage.Section,
			Action.Montage.PlayRate,
			Action.Montage.bLoop ? TEXT("Yes") : TEXT("No"));
	}

	return true;
}

void ULLMBlackboardMapper::ClearLLMKeys(UBlackboardComponent* Blackboard)
{
	if (!Blackboard)
	{
		return;
	}

	Blackboard->ClearValue(KEY_Intent);
	Blackboard->ClearValue(KEY_TargetLocation);
	Blackboard->ClearValue(KEY_TargetActor);
	Blackboard->ClearValue(KEY_TargetId);
	Blackboard->ClearValue(KEY_TargetType);
	Blackboard->ClearValue(KEY_SpeakText);
	Blackboard->ClearValue(KEY_Confidence);
	// Clear montage-related keys if they exist
	Blackboard->ClearValue(KEY_MontageName);
	Blackboard->ClearValue(KEY_MontageSection);
	Blackboard->ClearValue(KEY_MontagePlayRate);
	Blackboard->ClearValue(KEY_MontageLoop);
}

FString ULLMBlackboardMapper::GetRequiredBlackboardKeysDescription()
{
	return TEXT(
		"Required Blackboard Keys for BB_LLM:\n\n"
		"1. Intent (String) - The action intent: 'MoveTo', 'Interact', 'Speak', 'PlayMontage', 'Idle'\n"
		"2. TargetLocation (Vector) - World position for MoveTo actions\n"
		"3. TargetActor (Object) - Actor reference for Interact actions\n"
		"4. TargetId (String) - Target identifier or NavPoint name\n"
		"5. TargetType (String) - Target type (e.g., 'NPC', 'Door')\n"
		"6. SpeakText (String) - Text to speak for Speak actions\n"
		"7. Confidence (Float) - Action confidence score (0.0-1.0)\n"
		"8. MontageName (String) - Montage logical name to play (for PlayMontage)\n"
		"9. MontageSection (String) - Optional section name (for PlayMontage)\n"
		"10. MontagePlayRate (Float) - Play rate (for PlayMontage)\n"
		"11. MontageLoop (Bool) - Loop flag (for PlayMontage)\n"
	);
}
