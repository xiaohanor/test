// LLM Action data structures for controlling AI behavior
#pragma once

#include "CoreMinimal.h"
#include "LLMActionTypes.generated.h"

/**
 * Intent types supported by the LLM action system
 * Limited to MVP scope: MoveTo, Interact, Speak
 */
UENUM(BlueprintType)
enum class ELLMIntent : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	MoveTo UMETA(DisplayName = "Move To"),
	Interact UMETA(DisplayName = "Interact"),
	Speak UMETA(DisplayName = "Speak")
};

/**
 * Target information for actions
 */
USTRUCT(BlueprintType)
struct FLLMTarget
{
	GENERATED_BODY()

	// Optional target ID (e.g., "NPC_1", "Door_Main")
	UPROPERTY(BlueprintReadWrite, Category = "LLM|Action")
	FString Id;

	// Optional target type (e.g., "NPC", "Door", "Item")
	UPROPERTY(BlueprintReadWrite, Category = "LLM|Action")
	FString Type;
};

/**
 * Location information for MoveTo actions
 * Can be either coordinates or a named navigation point
 */
USTRUCT(BlueprintType)
struct FLLMLocation
{
	GENERATED_BODY()

	// Direct world coordinates
	UPROPERTY(BlueprintReadWrite, Category = "LLM|Action")
	FVector Coordinates = FVector::ZeroVector;

	// Named navigation point (alternative to coordinates)
	UPROPERTY(BlueprintReadWrite, Category = "LLM|Action")
	FString NavPointName;

	// True if using coordinates, false if using NavPointName
	UPROPERTY(BlueprintReadWrite, Category = "LLM|Action")
	bool bUseCoordinates = true;
};

/**
 * Complete LLM action structure
 * Represents a parsed and validated action from LLM output
 */
USTRUCT(BlueprintType)
struct FLLMAction
{
	GENERATED_BODY()

	// The intent/action type
	UPROPERTY(BlueprintReadWrite, Category = "LLM|Action")
	ELLMIntent Intent = ELLMIntent::Idle;

	// Target for Interact actions (optional)
	UPROPERTY(BlueprintReadWrite, Category = "LLM|Action")
	FLLMTarget Target;

	// Location for MoveTo actions (optional)
	UPROPERTY(BlueprintReadWrite, Category = "LLM|Action")
	FLLMLocation Location;

	// Text to speak for Speak actions (optional)
	UPROPERTY(BlueprintReadWrite, Category = "LLM|Action")
	FString Speak;

	// Optional additional parameters as JSON string
	UPROPERTY(BlueprintReadWrite, Category = "LLM|Action")
	FString Params;

	// Confidence score 0-1, actions below threshold should not execute
	UPROPERTY(BlueprintReadWrite, Category = "LLM|Action")
	float Confidence = 0.0f;

	// Raw JSON string that was parsed (for debugging)
	UPROPERTY(BlueprintReadWrite, Category = "LLM|Action")
	FString RawJson;
};
