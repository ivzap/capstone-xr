#pragma once
#include <vector>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// async headers
#include "Async/Async.h"

// Include the glTFRuntime header
#include "glTFRuntime.h" 
#include "glTFRuntimeFunctionLibrary.h"
#include "glTFRuntimeAsset.h"
#include "glTFRuntimeAssetActorAsync.h"
#include "glTFRuntimeParser.h"
// Include http libs headers
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Http.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#include "MyActor.generated.h"


UCLASS()
class MYVRTEST_API AMyActor : public AActor
{
    GENERATED_BODY()
    //DECLARE_DELEGATE_OneParam(FglTFRuntimeStaticMeshAsync, UStaticMesh*);

public:
    AMyActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    void DownloadGLB();
    void OnGLBDownloaded(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    void LoadGLBFileAsync(const FString& FilePath);

    UFUNCTION()
    void OnStaticMeshLoaded(UStaticMesh* LoadedMesh);

    UFUNCTION(BlueprintCallable, Category = "GLB")
    void LoadGLBFile(const FString& FilePath);
    void LoadGLBMesh(UglTFRuntimeAsset* GLBAsset);

private:
    USceneComponent* RootComponent; // Declare the root component
    FTimerHandle HttpRequestTimer;
    std::vector<UStaticMeshComponent*> RootChildren;
    TArray<uint8> CurrentMeshBytes;

};
