#pragma once

#include "vehicles/car/api/CarApiBase.hpp"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "physics/Kinematics.hpp"
#include "CarPawnApi.h"

class ACarPawn;
class AProbotPawn;

class ProbotPawnApi : public CarPawnApi
{
public:
    typedef msr::airlib::ImageCaptureBase ImageCaptureBase;

    ProbotPawnApi(ACarPawn* pawn, const msr::airlib::Kinematics::State* pawn_kinematics,
                  msr::airlib::CarApiBase* vehicle_api);

    virtual void updateMovement(const msr::airlib::CarApiBase::CarControls& controls) override;

    virtual msr::airlib::CarApiBase::CarState getCarState() const override;

    virtual void reset();

    virtual ~ProbotPawnApi();

protected:
    AProbotPawn* pawn_;

private:
    UChaosWheeledVehicleMovementComponent* movement_;
};
