#include "ProbotPawnApi.h"
#include "AirBlueprintLib.h"

#include "ProbotPawn.h"

ProbotPawnApi::ProbotPawnApi(ACarPawn* pawn, const msr::airlib::Kinematics::State* pawn_kinematics,
                             msr::airlib::CarApiBase* vehicle_api)
    : CarPawnApi(pawn, pawn_kinematics, vehicle_api)
{
    movement_ = CastChecked<UChaosWheeledVehicleMovementComponent>(pawn->GetVehicleMovement());
    pawn_ = static_cast<AProbotPawn*>(pawn);
}

void ProbotPawnApi::updateMovement(const msr::airlib::CarApiBase::CarControls& controls)
{
    last_controls_ = controls;

    ITnVehicleMotionModel::MotionControlInput controlInput;

    controlInput.validFields = static_cast<ITnVehicleMotionModel::EPossibleInputCommands>(ITnVehicleMotionModel::EPIC_STEERING | ITnVehicleMotionModel::EPIC_THROTTLE);
    controlInput.throttleCommand = FMath::Clamp(controls.throttle * 100, -pawn_->MaxThrottle, pawn_->MaxThrottle);
    controlInput.steeringCommand = FMath::Clamp(-controls.steering * 2 * 100, -pawn_->MaxSteering, pawn_->MaxSteering);
    pawn_->MotionModel->SetControlCommands(controlInput);

    /*
    if (!controls.is_manual_gear && movement_->GetTargetGear() < 0)
        movement_->SetTargetGear(0, true); //in auto gear we must have gear >= 0
    if (controls.is_manual_gear && movement_->GetTargetGear() != controls.manual_gear)
        movement_->SetTargetGear(controls.manual_gear, controls.gear_immediate);
        */
    movement_->SetThrottleInput(controlInput.throttleCommand);
    movement_->SetSteeringInput(controlInput.steeringCommand);
    // movement_->SetBrakeInput(controls.brake);
    // movement_->SetHandbrakeInput(controls.handbrake);
    // movement_->SetUseAutoGears(!controls.is_manual_gear);
}

msr::airlib::CarApiBase::CarState ProbotPawnApi::getCarState() const
{
    msr::airlib::CarApiBase::CarState state(
        pawn_->MotionModel->GetSpeed(),
        0 /*movement_->GetCurrentGear()*/,
        0 /*movement_->GetEngineRotationSpeed()*/,
        pawn_->MotionModel->GetRpmMax(),
        false, /*last_controls_.handbrake,*/
        *pawn_kinematics_,
        msr::airlib::ClockFactory::get()->nowNanos());

    return state;
}

void ProbotPawnApi::reset()
{
    CarPawnApi::reset();

    pawn_->WorldToGlobalOffset = FVector(pawn_->GetInitPosition().y, pawn_->GetInitPosition().x, pawn_->GetInitPosition().z) * 100;
    pawn_->InitModel(pawn_->GetInitPosition(), pawn_->GetInitYaw());
}

ProbotPawnApi::~ProbotPawnApi() = default;
