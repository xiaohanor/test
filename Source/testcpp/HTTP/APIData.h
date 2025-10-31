// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "APIData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class TESTCPP_API UAPIData : public UDataAsset
{
	GENERATED_BODY()

public:
	// Simple getters to access config at runtime
	UFUNCTION(BlueprintPure, Category="API Data")
	FString GetAPIName() const { return Name; }

	UFUNCTION(BlueprintPure, Category="API Data")
	FString GetURL() const { return URL; }

	UFUNCTION(BlueprintPure, Category="API Data")
	FString GetAPIKey() const { return APIKey; }

	UFUNCTION(BlueprintPure, Category="API Data")
	FString GetModel() const { return Model; }

protected:
	// Name of the API, not used in code
	UPROPERTY(EditDefaultsOnly, Category="API Data")
	FString Name;

	UPROPERTY(EditDefaultsOnly, Category="API Data")
	FString URL;
	
	UPROPERTY(EditDefaultsOnly, Category="API Data")
	FString APIKey;

	UPROPERTY(EditDefaultsOnly, Category="API Data")
	FString Model;
	
};
