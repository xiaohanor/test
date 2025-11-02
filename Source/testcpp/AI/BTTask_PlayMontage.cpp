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

	// Get montage name and params from blackboard
	FString MontageName = BlackboardComp->GetValueAsString(MontageNameKey.SelectedKeyName);
	if (MontageName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BTTask_PlayMontage] MontageName is empty"));
		return EBTNodeResult::Failed;
	}
	FString SectionName = BlackboardComp->GetValueAsString(MontageSectionKey.SelectedKeyName);
	float PlayRate = BlackboardComp->GetValueAsFloat(MontagePlayRateKey.SelectedKeyName);
	if (PlayRate <= 0.f) { PlayRate = 1.f; }
	bool bLoop = BlackboardComp->GetValueAsBool(MontageLoopKey.SelectedKeyName);

	// Resolve montage from mapping (case-insensitive)
	UAnimMontage* MontageAsset = nullptr;
	for (const FNamedMontage& Entry : MontageMap)
	{
		if (!Entry.Montage.IsNull() && Entry.Name.Equals(MontageName, ESearchCase::IgnoreCase))
		{
			MontageAsset = Entry.Montage.LoadSynchronous();
			break;
		}
	}

	if (!MontageAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("[BTTask_PlayMontage] No montage asset mapped for name: %s"), *MontageName);
		return EBTNodeResult::Failed;
	}

	// Play montage
	float PlayedLen = AnimInstance->Montage_Play(MontageAsset, PlayRate);
	if (PlayedLen <= 0.f)
	{
		UE_LOG(LogTemp, Error, TEXT("[BTTask_PlayMontage] Montage_Play failed for: %s"), *MontageAsset->GetName());
		return EBTNodeResult::Failed;
	}

	if (!SectionName.IsEmpty())
	{
		AnimInstance->Montage_JumpToSection(FName(*SectionName), MontageAsset);
	}

	// Basic loop support: if requested, set next section to itself when possible
	if (bLoop)
	{
		// This is a best-effort approach; for complex montages, designers should set up looping in asset
		// Try to set the first section to loop to itself
		if (MontageAsset->CompositeSections.Num() > 0)
		{
			FName FirstSection = MontageAsset->CompositeSections[0].SectionName;
			AnimInstance->Montage_SetNextSection(FirstSection, FirstSection, MontageAsset);
		}
	}

	FString CharacterName = Character->GetName();
	UE_LOG(LogTemp, Log, TEXT("[BTTask_PlayMontage] %s playing montage: %s (Section: %s, Rate: %.2f, Loop: %s)"), 
		*CharacterName, *MontageName, 
		SectionName.IsEmpty() ? TEXT("None") : *SectionName,
		PlayRate,
		bLoop ? TEXT("Yes") : TEXT("No"));

	if (GEngine)
	{
		FString DisplayText = FString::Printf(TEXT("%s: Playing montage '%s'"), *CharacterName, *MontageName);
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, DisplayText);
	}

	// MVP: Do not wait; return succeeded after start
	return EBTNodeResult::Succeeded;
}

FString UBTTask_PlayMontage::GetStaticDescription() const
{
	return FString::Printf(TEXT("Play animation montage from blackboard\nMontage Key: %s\nSection Key: %s\nPlay Rate Key: %s\nLoop Key: %s\nWait for Finish: %s\nMapping Count: %d"),
		*MontageNameKey.SelectedKeyName.ToString(),
		*MontageSectionKey.SelectedKeyName.ToString(),
		*MontagePlayRateKey.SelectedKeyName.ToString(),
		*MontageLoopKey.SelectedKeyName.ToString(),
		bWaitForFinish ? TEXT("Yes") : TEXT("No"),
		MontageMap.Num());
}
