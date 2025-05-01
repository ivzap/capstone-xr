#include "DisplayTextActor.h"


ADisplayTextActor::ADisplayTextActor()
{
    PrimaryActorTick.bCanEverTick = false;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    TextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextRender"));
    TextComponent->SetupAttachment(RootComponent);
    TextComponent->SetHorizontalAlignment(EHTA_Center);
    TextComponent->SetWorldSize(50.f);
    TextComponent->SetText(FText::FromString(TEXT("")));
    TextComponent->SetWorldSize(10.0f); // Set font size to 50 units
}

void ADisplayTextActor::PollText() {
    FString Url = "http://147.185.221.26:7257/model.txt";
    FHttpModule* Http = &FHttpModule::Get();

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
    Request->SetURL(Url);
    Request->SetVerb("GET");
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/octet-stream"));

    Request->OnProcessRequestComplete().BindUObject(this, &ADisplayTextActor::OnTextDownload);
    Request->ProcessRequest();
}

void ADisplayTextActor::OnTextDownload(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (!bWasSuccessful || !Response.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to download text from URL"));
        return;
    }

    FString ResponseString = Response->GetContentAsString();

    UE_LOG(LogTemp, Log, TEXT("Downloaded text: %s"), *ResponseString);

    if (TextComponent)
    {
        TextComponent->SetText(FText::FromString(ResponseString));
    }
}

void ADisplayTextActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    /*APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC && PC->PlayerCameraManager)
    {
        FVector ToCamera = PC->PlayerCameraManager->GetCameraLocation() - GetActorLocation();
        SetActorRotation(ToCamera.Rotation());
    }*/
}

void ADisplayTextActor::BeginPlay() {
    Super::BeginPlay(); // Always call parent class implementation first

    GetWorldTimerManager().SetTimer(HttpRequestTimer, this, &ADisplayTextActor::PollText, 5.0f, true);
}

void ADisplayTextActor::SetDisplayText(const FString& NewText)
{
    if (TextComponent)
    {
        TextComponent->SetText(FText::FromString(NewText));
    }
}
