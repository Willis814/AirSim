// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "AirSim.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"
#include <Interfaces/IPluginManager.h>

class FAirSim : public IModuleInterface
{
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    void* MotionCoreDllHandle;
};

IMPLEMENT_MODULE(FAirSim, AirSim)

void FAirSim::StartupModule()
{
    //plugin startup
    UE_LOG(LogTemp, Log, TEXT("StartupModule: AirSim plugin"));
    FString DepsDir = FPaths::Combine(IPluginManager::Get().FindPlugin("AirSim")->GetBaseDir(), TEXT("Source"), TEXT("AirLib"), TEXT("deps"));
    FString MotionPath = FPaths::Combine(DepsDir, TEXT("MotionCore"), TEXT("lib"));

    FString ControlDll;
    FString MotionDll;
    FString ext;

#if PLATFORM_WINDOWS
    FString winPath = FPaths::Combine(TEXT("x64"), TEXT("Release"));
    ext = TEXT(".dll");
    MotionDll = FPaths::Combine(winPath, TEXT("MotionCore"));
#elif PLATFORM_LINUX
    FString prefix = TEXT("lib");
    ext = TEXT(".so");
    MotionDll = prefix + TEXT("MotionCore");
#endif

    {
        FString DllPath = FPaths::Combine(MotionPath, MotionDll + ext);
        MotionCoreDllHandle = FPlatformProcess::GetDllHandle(*DllPath);
        if (!MotionCoreDllHandle) {
            UE_LOG(LogTemp, Error, TEXT("Failed to load MotionCore library."));
        }
        else {
            UE_LOG(LogTemp, Log, TEXT("MotionCore library loaded successfully."));
        }
    }
}

void FAirSim::ShutdownModule()
{
    //plugin shutdown
    FPlatformProcess::FreeDllHandle(MotionCoreDllHandle);
}