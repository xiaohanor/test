// Behavior Tree task to make character speak/display text
#include "AI/BTTask_Speak.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"

UBTTask_Speak::UBTTask_Speak()
{
	NodeName = "Speak";
	
	// Set default blackboard key
	SpeakTextKey.AddStringFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_Speak, SpeakTextKey));
}

EBTNodeResult::Type UBTTask_Speak::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		UE_LOG(LogTemp, Error, TEXT("[BTTask_Speak] No Blackboard component"));
		return EBTNodeResult::Failed;
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		UE_LOG(LogTemp, Error, TEXT("[BTTask_Speak] No AI Controller"));
		return EBTNodeResult::Failed;
	}

	APawn* ControlledPawn = AIController->GetPawn();

	// Get text from blackboard
	FString SpeakText = BlackboardComp->GetValueAsString(SpeakTextKey.SelectedKeyName);
	
	if (SpeakText.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BTTask_Speak] SpeakText is empty"));
		return EBTNodeResult::Failed;
	}

	// MVP implementation: Log the speech
	// In a full implementation, you would:
	// 1. Display subtitles on screen
	// 2. Play voice audio
	// 3. Trigger speech animations
	// 4. Wait for DisplayDuration before completing
	
	FString PawnName = ControlledPawn ? ControlledPawn->GetName() : TEXT("Unknown");
	UE_LOG(LogTemp, Log, TEXT("[BTTask_Speak] %s says: \"%s\""), *PawnName, *SpeakText);
	
	// For MVP, we'll use on-screen debug message as well
	if (GEngine)
	{
		FString DisplayText = FString::Printf(TEXT("%s: %s"), *PawnName, *SpeakText);
		GEngine->AddOnScreenDebugMessage(-1, DisplayDuration, FColor::Cyan, DisplayText);
	}

	return EBTNodeResult::Succeeded;
}

FString UBTTask_Speak::GetStaticDescription() const
{
	return FString::Printf(TEXT("Speak text from blackboard\nText Key: %s\nDisplay Duration: %.1fs"),
		*SpeakTextKey.SelectedKeyName.ToString(),
		DisplayDuration);
}
