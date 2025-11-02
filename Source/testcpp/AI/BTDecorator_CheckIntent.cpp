// Behavior Tree decorator to check LLM intent
#include "AI/BTDecorator_CheckIntent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTDecorator_CheckIntent::UBTDecorator_CheckIntent()
{
	NodeName = "Check Intent";
	
	// Set default blackboard key
	IntentKey.AddStringFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_CheckIntent, IntentKey));
	
	// Default expected intent
	ExpectedIntent = TEXT("MoveTo");
}

bool UBTDecorator_CheckIntent::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return false;
	}

	FString CurrentIntent = BlackboardComp->GetValueAsString(IntentKey.SelectedKeyName);
	bool bMatches = CurrentIntent.Equals(ExpectedIntent, ESearchCase::IgnoreCase);

	if (bMatches)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[BTDecorator_CheckIntent] Intent '%s' matches expected '%s'"), 
			*CurrentIntent, *ExpectedIntent);
	}

	return bMatches;
}

FString UBTDecorator_CheckIntent::GetStaticDescription() const
{
	return FString::Printf(TEXT("Check if Intent == '%s'"), *ExpectedIntent);
}
