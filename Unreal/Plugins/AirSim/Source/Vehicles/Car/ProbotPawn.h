#pragma once
#pragma warning(push)
#pragma warning(disable : 4458)

#include "CoreMinimal.h"
#include "CarPawn.h"

THIRD_PARTY_INCLUDES_START
#include <MotionCore/ITnAppItem.h>
#include <MotionCore/ITnMotionQueries.h>
#include <MotionCore/ITnPhysicalItemBinder.h>
THIRD_PARTY_INCLUDES_END

#include <MaterialMapping.h>

#include "ProbotPawn.generated.h"

class UDataTable;

class UUnrealDTMSensor;
class ITnPhysicalItem;
class ITnVehicleMotionModel;

class UnrealAppItem : public ITnAppItem
{
public:
    UnrealAppItem(int idx)
    {
        Index = idx;
    }

    int Index;
};

UENUM(BlueprintType)
enum class EModelType : uint8
{
    None,
    Probot,
    Rook
};

UCLASS(config = Game)
class AProbotPawn : public ACarPawn
    , public ITnWheeledVehicleMotionModelListener
    , public ITnMotionModelSafetyProblemReport
    , public ITnMotionQueries
    , public ITnPhysicalItemBinder
{
    GENERATED_BODY()

public:
    AProbotPawn();

    virtual void BeginPlay() override;
    virtual void Tick(float Delta) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation,
                           FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

    // ITnPhysicalItemBinder override
    virtual void Bind(ITnPhysicalItem*) override;
    virtual void OnUpdate(ITnPhysicalItem** pITnPhysicalItemsArray, int numItems) override;
    bool OnCollision(ITnCollisionPointPhysicalItem** pITnCollisionPointsArray, int numItems) override;

    // ITnMotionQueries override
    virtual void GetTerrainHeight(double x, double y, bool* isHeightFound, double* pdHeight) override;
    virtual void GetTerrainMaterial(const STnVector3D& WorldPos, bool* bpMaterialFound, ITnMotionMaterial::STerrainMaterialType& TerrainMaterialType) override;
    virtual void GetTerrainMoisture(const STnVector3D& WorldPos, bool* bpMoistureFound, double& moisture) override;

    // ITnWheeledVehicleMotionModelListener override
    virtual void StartTimer() override {}
    virtual void OnDataUpdate(double timeSeconds) override {}
    virtual double GetTimeSeconds() override { return 0; }

    // ITnMotionModelSafetyProblemReport override
    virtual void SafetyEvent(ITnErrors::EMotionCode SafetyProblem) override;

    // ACarPawn override
    virtual void updateHUDStrings() override;

    void InitModel(const STnVector3D Position, const double Yaw);
    void LoadMaterialMappingTable();

    STnVector3D GetInitPosition() const;
    double GetInitYaw() const;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UUnrealDTMSensor* DTMSensor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<UStaticMeshComponent*> PlatformComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EModelType ModelType;

    /** Max Throttle [%] */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, ClampMax = 100))
    float MaxThrottle;

    /** Max Steering [%] */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, ClampMax = 100))
    float MaxSteering;

    ITnVehicleMotionModel* MotionModel;
    FVector WorldToGlobalOffset;

private:
    inline void DoPhysics(float DeltaTime);

    void PreFirstUpdate();
    void PostFirstUpdate();

    void SetInitPosition(STnVector3D Position);
    void SetInitYaw(double Yaw);

private:
    float VehicleSpeed;
    float SlowMoFactor;
    STnVector3D InitPos;
    double InitYaw;
    bool isMaterialMappingFound;
    bool bSweep;
    bool bCheckInInitPos;

    UPROPERTY()
    UDataTable* material_mapping_table;

    TMap<FString, TTuple<ETerrainType, ETerrainSubType>> MaterialMapping;
};

static const char* safetyEnumStr[] = {
    "MAX_SIDE_SLOP_DETECTED",
    "MAX_RISE_SLOP_DETECTED",
    "MAX_DOWN_SLOP_DETECTED",
    "MAX_SLIP_PERCENT_DETECTED",
    "MAX_DELTA_WHEEL_DETECTED",
    "MAX_NEGATIVE_VELOCITY_DETECTED",
    "MAX_DEVIATION_FROM_PATH_DETECTED",
    "COLLISION_DETECTED",
    "EXTERNAL_COLLISION_DETECTED",
    "TERRAIN_HEIGHT_PROBLEM_DETECTED",
    "UNTRAVERSABLE_MATERIAL_DETECTED"
};

#pragma warning(pop)