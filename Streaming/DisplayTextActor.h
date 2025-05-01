// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
// https modules
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Http.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#include "Components/TextRenderComponent.h"

#include "DisplayTextActor.generated.h"


UCLASS()
class MYVRTEST_API ADisplayTextActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADisplayTextActor();
	void PollText();
	void SetDisplayText(const FString& NewText);
	void OnTextDownload(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	FTimerHandle HttpRequestTimer;
	UPROPERTY(VisibleAnywhere)
	UTextRenderComponent* TextComponent;

};

//void ADisplayTextActor::SetDisplayText(const FString& NewText)
//{
//	if (TextComponent)
//	{
//		TextComponent->SetText(FText::FromString(NewText));
//	}
//}
