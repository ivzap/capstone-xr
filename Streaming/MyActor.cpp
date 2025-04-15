#include "MyActor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

AMyActor::AMyActor()
{
    PrimaryActorTick.bCanEverTick = false;
    RootComponent = CreateDefaultSubobject<USceneComponent>("Root");
    SetRootComponent(RootComponent);
}

void AMyActor::BeginPlay()
{
    Super::BeginPlay(); // Always call parent class implementation first
    
    GetWorldTimerManager().SetTimer(HttpRequestTimer, this, &AMyActor::DownloadGLB, 5.0f, true);

    // Optional: Auto-load your GLB when actor spawns
    //LoadGLBFile("C:/Users/zapla/OneDrive/Documents/Unreal Projects/MyProject1/Content/sample.glb");
}

void AMyActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime); // Always call parent class implementation first

    // Optional: Add any per-frame logic here
    // For example, you could rotate your loaded mesh:
    /*
    if (RootComponent && RootComponent->GetNumChildrenComponents() > 0)
    {
        UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(RootComponent->GetChildComponent(0));
        if (Mesh)
        {
            Mesh->AddRelativeRotation(FRotator(0, DeltaTime * 30.0f, 0));
        }
    }
    */
}

void AMyActor::DownloadGLB()
{
    FString Url = "http://127.0.0.1:5000/model.glb";
    FHttpModule* Http = &FHttpModule::Get();

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
    Request->SetURL(Url);
    Request->SetVerb("GET");
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/octet-stream"));

    Request->OnProcessRequestComplete().BindUObject(this, &AMyActor::OnGLBDownloaded);
    Request->ProcessRequest();
}

void AMyActor::OnGLBDownloaded(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (!bWasSuccessful || !Response.IsValid()) return;

    const TArray<uint8>& Data = Response->GetContent();
    // dont want same object to display, expensive operation
    if (Data == CurrentMeshBytes) {
        return;
    }

    CurrentMeshBytes = Data;

    for (USceneComponent* Child : RootChildren)
    {
        if (Child)
        {
            Child->UnregisterComponent();
            Child->DestroyComponent();
            Child = nullptr;
        }
    }

    RootChildren = {};

    FString SavePath = FPaths::ProjectPersistentDownloadDir() + TEXT("DownloadedModel.glb");

    if (FFileHelper::SaveArrayToFile(Data, *SavePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("Saved .glb to: %s"), *SavePath);
        // TODO: Now load and display it!
        LoadGLBFileAsync(SavePath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save .glb file"));
    }
}

void AMyActor::LoadGLBFile(const FString& FilePath)
{
    // 1. Create minimal config
    FglTFRuntimeConfig LoaderConfig;
    LoaderConfig.bAllowExternalFiles = false;

    // 2. Load the asset
    UglTFRuntimeAsset* GLBAsset = UglTFRuntimeFunctionLibrary::glTFLoadAssetFromFilename(
        FilePath,
        false, // bPathRelativeToContent
        LoaderConfig
    );

    if (!GLBAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load GLB"));
        return;
    }

    // 3. Get the first mesh from the asset
    FglTFRuntimeStaticMeshConfig MeshConfig;
    MeshConfig.bBuildSimpleCollision = true;
    UE_LOG(LogTemp, Log, TEXT("Getting ready to load mesh!"));

    UStaticMesh* LoadedMesh = GLBAsset->LoadStaticMesh(0, MeshConfig);
    if (LoadedMesh)
    {
        // 2. Destroy each child component
        for (UStaticMeshComponent* Child : RootChildren)
        {
            UE_LOG(LogTemp, Warning, TEXT("Destroying child component: %s"), *Child->GetName());
            if (Child)
            {
                Child->UnregisterComponent();
                Child->DestroyComponent();
                UE_LOG(LogTemp, Log, TEXT("Destroyed child component: %s"), *Child->GetName());
            }
        }

        // 4. Create and setup mesh component
        UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(this);
        UE_LOG(LogTemp, Warning, TEXT("New object created!"));
        RootChildren.push_back(MeshComp);

        MeshComp->SetStaticMesh(LoadedMesh);
        MeshComp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
        MeshComp->SetRelativeScale3D(FVector(5.0f)); // glTF often needs scaling
        MeshComp->SetRelativeLocation(FVector::ZeroVector);
        MeshComp->SetRelativeRotation(FRotator::ZeroRotator);
        // Mobility
        MeshComp->SetMobility(EComponentMobility::Movable);
        MeshComp->SetSimulatePhysics(true); // Enable physics simulation
        MeshComp->SetNotifyRigidBodyCollision(true); // Enable hit events if needed
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // Enable collision
        MeshComp->SetCollisionObjectType(ECC_PhysicsBody); // Set as physics object

        MeshComp->RegisterComponent();
    }
}

void AMyActor::OnStaticMeshLoaded(UStaticMesh* LoadedMesh)
{
    if (!LoadedMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("Async mesh load failed"));
        return;
    }

    // Destroy old components
    /*for (USceneComponent* Child : RootChildren)
    {
        if (Child)
        {
            Child->UnregisterComponent();
            Child->DestroyComponent();
        }
    }*/

    // Attach new mesh
    UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(this);
    RootChildren.push_back(MeshComp);
    MeshComp->SetStaticMesh(LoadedMesh);
    MeshComp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
    MeshComp->SetRelativeScale3D(FVector(5.0f));
    MeshComp->SetMobility(EComponentMobility::Movable);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComp->RegisterComponent();

    UE_LOG(LogTemp, Log, TEXT("GLB mesh loaded and attached (async)"));
}


void AMyActor::LoadGLBFileAsync(const FString& FilePath)
{
  
    // Part 1: Load the GLB asset in background
    FglTFRuntimeConfig LoaderConfig;
    LoaderConfig.bAllowExternalFiles = false;

    UglTFRuntimeAsset* GLBAsset = UglTFRuntimeFunctionLibrary::glTFLoadAssetFromFilename(
        FilePath,
        false, // bPathRelativeToContent
        LoaderConfig
    );

    if (!GLBAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load GLB"));
        return;
    }

    LoadGLBMesh(GLBAsset);
          
}

void AMyActor::LoadGLBMesh(UglTFRuntimeAsset* GLBAsset)
{
    // Must be on Game Thread
    FglTFRuntimeStaticMeshConfig MeshConfig;
    MeshConfig.bBuildSimpleCollision = true;

    UE_LOG(LogTemp, Log, TEXT("Loading StaticMesh from GLBAsset..."));

    // 2. Create delegate the correct way
    FglTFRuntimeStaticMeshAsync Callback;
    Callback.BindDynamic(this, &AMyActor::OnStaticMeshLoaded);

    // 3. Call async load
    GLBAsset->LoadStaticMeshAsync(
        0,               // mesh index
        Callback,   // delegate
        MeshConfig
    );
}