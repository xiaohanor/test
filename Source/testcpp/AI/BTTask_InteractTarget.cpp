// Behavior Tree task to interact with a target actor
#include "AI/BTTask_InteractTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "GameFramework/Actor.h"

UBTTask_InteractTarget::UBTTask_InteractTarget()
{
	NodeName = "Interact Target";
	
	// Set default blackboard keys
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_InteractTarget, TargetActorKey), AActor::StaticClass());
	TargetIdKey.AddStringFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_InteractTarget, TargetIdKey));
	TargetTypeKey.AddStringFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_InteractTarget, TargetTypeKey));
}

EBTNodeResult::Type UBTTask_InteractTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		UE_LOG(LogTemp, Error, TEXT("[BTTask_InteractTarget] No Blackboard component"));
		return EBTNodeResult::Failed;
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		UE_LOG(LogTemp, Error, TEXT("[BTTask_InteractTarget] No AI Controller"));
		return EBTNodeResult::Failed;
	}

	APawn* ControlledPawn = AIController->GetPawn();

	// Try to get target actor from blackboard
	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
	
	if (TargetActor)
	{
		// MVP implementation: Just log the interaction
		// In a full implementation, you would:
		// 1. Check if TargetActor implements IInteractable interface
		// 2. Call the interaction method
		// 3. Play animations, sounds, etc.
		
		UE_LOG(LogTemp, Log, TEXT("[BTTask_InteractTarget] Interacting with actor: %s"), *TargetActor->GetName());
		
		// Example interface call (commented out for MVP):
		// IInteractable* Interactable = Cast<IInteractable>(TargetActor);
		// if (Interactable)
		// {
		//     Interactable->Interact(ControlledPawn);
		// }
		
		return EBTNodeResult::Succeeded;
	}
	else
	{
		// Fallback: try to use TargetId or TargetType to find an actor
		FString TargetId = BlackboardComp->GetValueAsString(TargetIdKey.SelectedKeyName);
		FString TargetType = BlackboardComp->GetValueAsString(TargetTypeKey.SelectedKeyName);
		
		if (!TargetId.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("[BTTask_InteractTarget] TargetActor not set, but TargetId='%s' found. Need to resolve to actor."), *TargetId);
			// In a full implementation, you would query a registry or world to find the actor by ID
		}
		
		if (!TargetType.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("[BTTask_InteractTarget] Could search for nearest actor of type '%s'"), *TargetType);
			// In a full implementation, you would find the nearest interactable of this type
		}
		
		UE_LOG(LogTemp, Error, TEXT("[BTTask_InteractTarget] No valid target to interact with"));
		return EBTNodeResult::Failed;
	}
}

FString UBTTask_InteractTarget::GetStaticDescription() const
{
	return FString::Printf(TEXT("Interact with target actor from blackboard\nActor Key: %s\nID Key: %s\nType Key: %s"),
		*TargetActorKey.SelectedKeyName.ToString(),
		*TargetIdKey.SelectedKeyName.ToString(),
		*TargetTypeKey.SelectedKeyName.ToString());
}
