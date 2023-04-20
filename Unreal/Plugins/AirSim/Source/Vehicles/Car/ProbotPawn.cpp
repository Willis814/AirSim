#include "ProbotPawn.h"

#include <Interfaces/IPluginManager.h>

#include "MaterialMapping.h"

#include <MotionCore/ITnMotionCore.h>
#include <MotionCore/ITnPhysicalItem.h>
#include <MotionCore/ITnVehicleMotionModel.h>

#define LOCTEXT_NAMESPACE "VehiclePawn"

AProbotPawn::AProbotPawn()
    : ModelType(EModelType::None)
    , MaxThrottle(10.0)
    , MaxSteering(10.0)
    , MotionModel(nullptr)
    , VehicleSpeed(2)
    , SlowMoFactor(1)
    , isMaterialMappingFound(false)
    , bSweep(false)
    , bCheckInInitPos(true)
{
    LoadMaterialMappingTable();
}

void AProbotPawn::BeginPlay()
{
    Super::BeginPlay();

    if (MotionModel == nullptr) {
        MotionModel = ITnMotionCore::CreateVehicleMotionModel3D(this, this);
        MotionModel->SetUpdateListener(this);
        MotionModel->SetSafetyListener(this);

        FString baseDir = FPaths::Combine(IPluginManager::Get().FindPlugin("AirSim")->GetBaseDir(), TEXT("/Source/AirLib/deps/MotionCore/"));
        FString configFilename;

        switch (ModelType) {
        case EModelType::None:
            UAirBlueprintLib::LogMessageString("ProbotPawn: ModelType can't be None", "", LogDebugLevel::Failure);
            UAirBlueprintLib::ShowMessage(EAppMsgType::Ok, "ProbotPawn: ModelType can't be None", "Error");
            break;
        case EModelType::Probot:
            configFilename = "vehicleDataProbot.yaml";
            break;
        case EModelType::Rook:
            configFilename = "vehicleDataRook.yaml";
            break;
        case EModelType::Ford350:
            configFilename = "vehicleDataFord350.yaml";
            break;
        case EModelType::Tomcar:
            configFilename = "vehicleDataTomcar.yaml";
            break;
        case EModelType::Hummer:
            configFilename = "vehicleDataHummer.yaml";
            break;
        }

        FString configFilePath = FPaths::Combine(baseDir, configFilename);
        if (!FPaths::FileExists(configFilePath)) {
            std::string msg = std::string("Couldn't find config file: ") + TCHAR_TO_UTF8(*configFilePath);
            UAirBlueprintLib::LogMessageString(msg, "", LogDebugLevel::Failure);
            UAirBlueprintLib::ShowMessage(EAppMsgType::Ok, msg, "Error");
        }

        bool isReload = false;
        ITnMotionModel::MotionModelGenerateData generateData;
        generateData.collisionPolicy = E_COLLISION_POLICY::ECP_STOP_UPDATE_CYCLES;
        generateData.collisionEventsSource = E_COLLISION_EVENTS_SOURCE::ECE_EXTERNAL_EVENTS;
        generateData.terrainMateialSource = E_TERRAIN_MATERIAL_SOURCE::ETMS_USE_TERRAIN_MATERIAL_LAYER;
        bool ret = MotionModel->Generate(TCHAR_TO_ANSI(*configFilePath), isReload, generateData);
        if (!ret) {
            UAirBlueprintLib::LogMessageString("Motion Model couldn't be generated", "", LogDebugLevel::Failure);
        }

        SetInitPosition(STnVector3D(GetActorLocation().Y, GetActorLocation().X, GetActorLocation().Z) / 100);
        WorldToGlobalOffset = FVector(GetInitPosition().y, GetInitPosition().x, GetInitPosition().z) * 100;

        SetInitYaw((double)GetRootComponent()->GetComponentRotation().Yaw);
        InitModel(GetInitPosition(), GetInitYaw());
    }

    AddTickPrerequisiteComponent(DTMSensor);
}

void AProbotPawn::Tick(float Delta)
{
    DoPhysics(Delta);
    Super::Tick(Delta);
    pawn_events_.getPawnTickSignal().emit(Delta);
}

void AProbotPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
}

void AProbotPawn::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation,
                            FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
    pawn_events_.getCollisionSignal().emit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

    HitLocation /= 100;
    MotionModel->SetCollisionEvent(STnVector3D(HitLocation.Y, HitLocation.X, HitLocation.Z));
}

void AProbotPawn::DoPhysics(float DeltaTime)
{
    MotionModel->Update(DeltaTime * SlowMoFactor);
}

void AProbotPawn::PreFirstUpdate(const STnVector3D& Position)
{
    bSweep = false;
    bCheckInInitPos = true;
    SetTeleportHeight(Position.z);
}

void AProbotPawn::PostFirstUpdate()
{
    bSweep = true;
    bCheckInInitPos = false;
    SetIsTeleport(false);
}

void AProbotPawn::Bind(ITnPhysicalItem* pItem)
{
    ITnPhysicalItem::EPhysicalItemType eType;
    eType = pItem->GetType();
    const char* itemID = pItem->GetTag();
    ITnAppItem* pAppItem;

    if (eType == ITnPhysicalItem::EPIT_CP) { // Collision point
        ;
    }
    else {
        for (int idx = 0; idx < PlatformComponents.Num(); idx++) {
            UStaticMeshComponent* pMesh = PlatformComponents[idx];

            if (pMesh != nullptr &&
                pMesh->ComponentTags.Contains(itemID)) {
                pAppItem = new UnrealAppItem(idx);
                pItem->SetAppItem(pAppItem);
            }
        }
    }
}

void AProbotPawn::OnUpdate(ITnPhysicalItem** pITnPhysicalItemsArray, int numItems)
{
    for (int i = 0; i < numItems; i++) {
        ITnPhysicalItem* pITnPhysicalItem = pITnPhysicalItemsArray[i];

        if (pITnPhysicalItem->AppItemExists()) {
            ITnAppItem* pAppItem = pITnPhysicalItem->GetAppItem();
            UnrealAppItem* pUnrealItem = (UnrealAppItem*)pAppItem;
            UStaticMeshComponent* pSaticMesh = PlatformComponents[pUnrealItem->Index];

            const char* tag = pITnPhysicalItem->GetTag();
            ITnPhysicalItem::EPhysicalItemType eType = pITnPhysicalItem->GetType();
            STnVector3D ItemPosition;
            STnRotation ItemRotation;

            if (eType == ITnPhysicalItem::EPIT_WHEEL || eType == ITnPhysicalItem::EPIT_ARM || eType == ITnPhysicalItem::EPIT_SPRING) {
                ItemPosition = pITnPhysicalItem->GetRelativePosition();
                ItemRotation = pITnPhysicalItem->GetRelativeRotation();
            }
            else {
                ItemPosition = pITnPhysicalItem->GetGlobalPosition();
                ItemRotation = pITnPhysicalItem->GetGlobalRotation();
            }

            ItemPosition *= 100;
            FVector Location = FVector(ItemPosition.y, ItemPosition.x, ItemPosition.z);
            FRotator Rotation = FRotator(ItemRotation.fPitch, ItemRotation.fYaw, ItemRotation.fRoll);

            if (eType == ITnPhysicalItem::EPIT_WHEEL || eType == ITnPhysicalItem::EPIT_ARM || eType == ITnPhysicalItem::EPIT_SPRING) {
                pSaticMesh->SetRelativeLocation(Location, bSweep);
                pSaticMesh->SetRelativeRotation(Rotation, bSweep);
            }
            else {
                pSaticMesh->SetWorldLocation(Location + WorldToGlobalOffset, bSweep);
                pSaticMesh->SetWorldRotation(Rotation, bSweep);
            }

            if (eType == ITnPhysicalItem::EPIT_SPRING) {
                ITnSpringPhysicalItem* pITnSpringPhysicalItem = pITnPhysicalItem->CastToSpringPhysicalItem();
                STnVector3D Scale = pITnSpringPhysicalItem->GetScale();
                pSaticMesh->SetWorldScale3D(FVector(Scale.y, Scale.x, Scale.z));
            }

            if (FString(tag).Equals("CHASSIS")) {
                // We need to update location and rotation of the root CarPawn (AirSim) component
                // to make AirSim features work in this platform.
                // This is a workaround to "attach" the root to the chassis, bc inherited component can't be moved.
                GetRootComponent()->SetWorldLocationAndRotation(Location + WorldToGlobalOffset, Rotation);
            }
        }
    }

    PostFirstUpdate();
}

bool AProbotPawn::OnCollision(ITnCollisionPointPhysicalItem** pITnCollisionPointsArray, int numItems)
{
    return false;
}

void AProbotPawn::GetTerrainHeightArray(STnVector2D*& WorldPos_Array, bool*& bpHeightFound_Array, double*& pdHeight_Array, int numPoints)
{
    ParallelFor(numPoints, [&](int32 idx) {
        GetTerrainHeight(WorldPos_Array[idx].x, WorldPos_Array[idx].y, &bpHeightFound_Array[idx], &pdHeight_Array[idx]);
    });
}

void AProbotPawn::GetTerrainHeight(double x, double y, bool* isFound, double* pdHeight)
{
    Exchange(x, y);
    x *= 100.0;
    y *= 100.0;
    *isFound = true;

    *pdHeight = DTMSensor->GetTerrainHeight(x, y, bCheckInInitPos, GetIsTeleport(), GetTeleportHeight()) / 100.0;
}

void AProbotPawn::GetTerrainMaterial(const STnVector3D& WorldPos, bool* bpMaterialFound, ITnMotionMaterial::STerrainMaterialType& TerrainMaterialType)
{
    *bpMaterialFound = false;
    if (!isMaterialMappingFound) {
        return;
    }

    FHitResult hitResult;
    FVector pos = this->GetActorLocation();
    FCollisionQueryParams queryParams;
    queryParams.AddIgnoredActor(this);
    bool isHitFound = this->GetWorld()->LineTraceSingleByChannel(hitResult, FVector(WorldPos.y * 100.0, WorldPos.x * 100.0, pos.Z), FVector(WorldPos.y * 100.0, WorldPos.x * 100.0, -HALF_WORLD_MAX), ECC_WorldStatic, queryParams);
    if (isHitFound) {
        UMaterialInterface* material = hitResult.Component->GetMaterial(0);
        if (IsValid(material)) {
            const auto& foundVal = MaterialMapping.Find(material->GetName());
            if (foundVal) {
                TerrainMaterialType.Type = (ITnMotionMaterial::ETerrainType)(uint8)foundVal->Key;
                TerrainMaterialType.SubType = (ITnMotionMaterial::ETerrainSubType)(uint8)foundVal->Value;
                *bpMaterialFound = true;
            }
        }
    }
}

void AProbotPawn::GetTerrainMoisture(const STnVector3D& WorldPos, bool* bpMoistureFound, double& moisture)
{
}

void AProbotPawn::SafetyEvent(ITnErrors::EMotionCode SafetyProblem)
{
    // subtract 201 because this is the value of the first Safety enum.
    // It may break easily but we don't have another way to print the enum
    // Refer to ITnErros.h, EMotionCode
    UAirBlueprintLib::LogMessageString("Motion Model Safety problem detected: ", safetyEnumStr[SafetyProblem - 201], LogDebugLevel::Failure, 15.0f);

    // emit dummy collision signal using airsim so we have indication using the API
    UPrimitiveComponent* Component(NULL);
    pawn_events_.getCollisionSignal().emit(Component, this, Component, true, FVector(EForceInit::ForceInitToZero), FVector(EForceInit::ForceInitToZero), FVector(EForceInit::ForceInitToZero), FHitResult());
}

void AProbotPawn::updateHUDStrings()
{
    float speed_unit_factor = msr::airlib::AirSimSettings::singleton().speed_unit_factor;
    FText speed_unit_label = FText::FromString(FString(msr::airlib::AirSimSettings::singleton().speed_unit_label.c_str()));
    float vel = FMath::Abs(MotionModel->GetSpeed());
    float vel_rounded = FMath::FloorToInt(vel * 10 * speed_unit_factor) / 10.0f;

    // Using FText because this is display text that should be localizable
    last_speed_ = FText::Format(LOCTEXT("SpeedFormat", "{0} {1}"), FText::AsNumber(vel_rounded), speed_unit_label);

    UAirBlueprintLib::LogMessage(TEXT("Speed: "), last_speed_.ToString(), LogDebugLevel::Informational);

    double RPM_R, RPM_L;
    MotionModel->GetEnginesRPM(RPM_R, RPM_L);
    UAirBlueprintLib::LogMessage(TEXT("RPM R: "), FText::AsNumber(RPM_R).ToString(), LogDebugLevel::Informational);
    UAirBlueprintLib::LogMessage(TEXT("RPM L: "), FText::AsNumber(RPM_L).ToString(), LogDebugLevel::Informational);
    UAirBlueprintLib::LogMessage(TEXT("Chassis Yaw: "), FText::AsNumber(MotionModel->GetVehicleYaw_LC()).ToString(), LogDebugLevel::Informational);
}

void AProbotPawn::InitModel(const STnVector3D Position, const double Yaw, const bool is_teleport)
{
    SetIsTeleport(is_teleport);
    PreFirstUpdate(Position);
    ITnErrors::EMotionCode ret = MotionModel->Init(Position, Yaw);
    if (ret == ITnErrors::EMotionCode::SUCCESS) {
        UAirBlueprintLib::LogMessageString("Motion Model initialized successfully", "", LogDebugLevel::Informational);
    }
    else {
        UAirBlueprintLib::LogMessageString("Motion Model couldn't be initialized", "", LogDebugLevel::Failure);
    }
}

void AProbotPawn::LoadMaterialMappingTable()
{
    material_mapping_table = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), NULL, TEXT("DataTable'/Game/MaterialMappingTable.MaterialMappingTable'")));
    if (!material_mapping_table) {
        UAirBlueprintLib::LogMessageString("Cannot find material mapping table. Use default.",
                                           "",
                                           LogDebugLevel::Informational);
    }
    else if (material_mapping_table->GetRowMap().Num() < 1) {
        UAirBlueprintLib::LogMessageString("Material mapping table is empty. Use default.",
                                           "",
                                           LogDebugLevel::Informational);
    }
    else {
        for (auto it : material_mapping_table->GetRowMap()) {
            FMaterialMapping* data = (FMaterialMapping*)(it.Value);
            MaterialMapping.Add(data->Material.GetAssetName(), TTuple<ETerrainType, ETerrainSubType>(data->TerrainType, data->TerrainSubType));
        }
        isMaterialMappingFound = true;
    }
}

STnVector3D AProbotPawn::GetInitPosition() const
{
    return InitPos;
}

double AProbotPawn::GetInitYaw() const
{
    return InitYaw;
}

float AProbotPawn::GetTeleportHeight() const
{
    return teleport_height_;
}

bool AProbotPawn::GetIsTeleport() const
{
    return is_teleport_;
}

void AProbotPawn::SetIsTeleport(bool val)
{
    is_teleport_ = val;
}

void AProbotPawn::SetTeleportHeight(float val)
{
    teleport_height_ = val;
}

void AProbotPawn::SetInitYaw(double Yaw)
{
    InitYaw = Yaw;
}

void AProbotPawn::SetInitPosition(STnVector3D Position)
{
    InitPos = Position;
}

#undef LOCTEXT_NAMESPACE