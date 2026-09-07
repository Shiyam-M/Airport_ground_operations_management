#ifndef AGOMS_EMERGENCY_LANDING_H
#define AGOMS_EMERGENCY_LANDING_H

#include <string>
#include "models/Enums.h"
#include "utils/DateTime.h"

namespace agoms {

// Record of an emergency-landing event for a flight.
class EmergencyLanding {
public:
    EmergencyLanding(std::string emergencyId, std::string flightId, std::string reason,
                      util::DateTime declaredAt);

    const std::string& getEmergencyId() const { return emergencyId_; }
    const std::string& getFlightId() const { return flightId_; }
    const std::string& getReason() const { return reason_; }
    util::DateTime getDeclaredAt() const { return declaredAt_; }
    util::DateTime getLandingTime() const { return landingTime_; }
    EmergencyStatus getStatus() const { return status_; }

    void recordLanding(util::DateTime landingTime);
    void updateStatus(EmergencyStatus status) { status_ = status; }

private:
    std::string emergencyId_;
    std::string flightId_;
    std::string reason_;
    util::DateTime declaredAt_;
    util::DateTime landingTime_;
    EmergencyStatus status_;
};

} // namespace agoms

#endif // AGOMS_EMERGENCY_LANDING_H
