// Behavior Tree task to play an animation montage on the AI character
#include "AI/BTTask_PlayMontage.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

UBTTask_PlayMontage::UBTTask_PlayMontage()
{
	NodeName = "Play Montage";
	
	// Set default blackboard keys
	MontageNameKey.AddStringFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_PlayMontage, MontageNameKey));
	MontageSectionKey.AddStringFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_PlayMontage, MontageSectionKey));
	MontagePlayRateKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_PlayMontage, MontagePlayRateKey));
	MontageLoopKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_PlayMontage, MontageLoopKey));
}

EBTNodeResult::Type UBTTask_PlayMontage::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		UE_LOG(LogTemp, Error, TEXT("[BTTask_PlayMontage] No Blackboard component"));
		return EBTNodeResult::Failed;
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		UE_LOG(LogTemp, Error, TEXT("[BTTask_PlayMontage] No AI Controller"));
		return EBTNodeResult::Failed;
	}

	ACharacter* Character = Cast<ACharacter>(AIController->GetPawn());
	if (!Character)
	{
		UE_LOG(LogTemp, Error, TEXT("[BTTask_PlayMontage] AI pawn is not a Character"));
		return EBTNodeResult::Failed;
	}

	UAnimInstance* AnimInstance = Character->GetMesh() ? Character->GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("[BTTask_PlayMontage] No AnimInstance on Character mesh"));
		return EBTNodeResult::Failed;
	}

	// Get montage name from blackboard
	FString MontageName = BlackboardComp->GetValueAsString(MontageNameKey.SelectedKeyName);
	if (MontageName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BTTask_PlayMontage] MontageName is empty"));
		return EBTNodeResult::Failed;
	}

	// For MVP, we'll use the montage name directly as a path hint
	// In production, you'd load montages via asset registry or maintain a montage mapping
	// For now, we'll just log and indicate success
	// The actual montage loading would need to be done via asset references
	
	// Get optional parameters
	FString SectionName = BlackboardComp->GetValueAsString(MontageSectionKey.SelectedKeyName);
	float PlayRate = BlackboardComp->GetValueAsFloat(MontagePlayRateKey.SelectedKeyName);
	// Parser has already validated and clamped PlayRate to [0.1, 5.0], trust it
	bool bLoop = BlackboardComp->GetValueAsBool(MontageLoopKey.SelectedKeyName);

	// MVP implementation: Log the montage play request
	// In a full implementation, you would:
	// 1. Load the montage asset by name or path
	// 2. Call AnimInstance->Montage_Play(Montage, PlayRate)
	// 3. If SectionName is set, call AnimInstance->Montage_JumpToSection(SectionName)
	// 4. If bLoop, set up montage looping
	// 5. If bWaitForFinish, bind to OnMontageEnded and return InProgress, complete later
	
	FString CharacterName = Character->GetName();
	UE_LOG(LogTemp, Log, TEXT("[BTTask_PlayMontage] %s playing montage: %s (Section: %s, Rate: %.2f, Loop: %s)"), 
		*CharacterName, *MontageName, 
		SectionName.IsEmpty() ? TEXT("None") : *SectionName,
		PlayRate,
		bLoop ? TEXT("Yes") : TEXT("No"));
	
	// For MVP, display on-screen debug message as visual feedback
	if (GEngine)
	{
		FString DisplayText = FString::Printf(TEXT("%s: Playing montage '%s'"), *CharacterName, *MontageName);
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, DisplayText);
	}

	// TODO: Production implementation would be:
	// FString MontagePath = FString::Printf(TEXT("/Game/Animations/%s.%s"), *MontageName, *MontageName);
	// UAnimMontage* Montage = LoadObject<UAnimMontage>(nullptr, *MontagePath);
	// if (Montage)
	// {
	//     float Duration = AnimInstance->Montage_Play(Montage, PlayRate);
	//     if (!SectionName.IsEmpty())
	//     {
	//         AnimInstance->Montage_JumpToSection(FName(*SectionName), Montage);
	//     }
	//     if (bLoop)
	//     {
	//         AnimInstance->Montage_SetNextSection(FName("Default"), FName("Default"), Montage);
	//     }
	//     if (bWaitForFinish)
	//     {
	//         // Bind to OnMontageEnded and return InProgress
	//         return EBTNodeResult::InProgress;
	//     }
	//     return EBTNodeResult::Succeeded;
	// }
	// else
	// {
	//     UE_LOG(LogTemp, Error, TEXT("[BTTask_PlayMontage] Failed to load montage: %s"), *MontagePath);
	//     return EBTNodeResult::Failed;
	// }

	return EBTNodeResult::Succeeded;
}

FString UBTTask_PlayMontage::GetStaticDescription() const
{
	return FString::Printf(TEXT("Play animation montage from blackboard\nMontage Key: %s\nSection Key: %s\nPlay Rate Key: %s\nLoop Key: %s\nWait for Finish: %s"),
		*MontageNameKey.SelectedKeyName.ToString(),
		*MontageSectionKey.SelectedKeyName.ToString(),
		*MontagePlayRateKey.SelectedKeyName.ToString(),
		*MontageLoopKey.SelectedKeyName.ToString(),
		bWaitForFinish ? TEXT("Yes") : TEXT("No"));
}
