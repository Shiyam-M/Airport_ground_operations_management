#include "models/EmergencyLanding.h"

namespace agoms {

EmergencyLanding::EmergencyLanding(std::string emergencyId, std::string flightId,
                                    std::string reason, util::DateTime declaredAt)
    : emergencyId_(std::move(emergencyId)),
      flightId_(std::move(flightId)),
      reason_(std::move(reason)),
      declaredAt_(declaredAt),
      landingTime_(static_cast<std::time_t>(0)),
      status_(EmergencyStatus::DECLARED) {}

void EmergencyLanding::recordLanding(util::DateTime landingTime) {
    landingTime_ = landingTime;
    status_ = EmergencyStatus::LANDED;
}

} // namespace agoms
