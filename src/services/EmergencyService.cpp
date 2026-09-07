#include "services/EmergencyService.h"
#include "utils/Exceptions.h"

namespace agoms {
namespace services {

EmergencyLanding EmergencyService::declareEmergency(const std::string& flightId, const std::string& reason) {
    // 1. Flight enters EmergencyLanding state (only legal from APPROACHING
    //    per the flight lifecycle state diagram).
    flightService_.transitionFlightStatus(flightId, FlightStatus::EMERGENCY_LANDING);

    // 2. Emergency landing details are recorded.
    std::string emergencyId = emergencyIdGen_.next();
    EmergencyLanding emergency(emergencyId, flightId, reason, util::DateTime::now());
    db_.saveEmergency(emergency);

    // 3. Normal resource allocation may be interrupted: callers should stop
    //    routing new non-emergency allocation requests to this flight's
    //    previously-planned resources until beginGroundHandling() runs;
    //    this is enforced procedurally by the ConsoleUI workflow rather
    //    than by AllocationService, since emergency resource needs vary
    //    case-by-case (see docs/design.md).
    return emergency;
}

EmergencyLanding EmergencyService::recordLanding(const std::string& emergencyId) {
    EmergencyLanding emergency = getEmergency(emergencyId);
    emergency.recordLanding(util::DateTime::now());
    db_.updateEmergency(emergency);
    return emergency;
}

void EmergencyService::beginGroundHandling(const std::string& flightId, const std::string& emergencyId) {
    EmergencyLanding emergency = getEmergency(emergencyId);
    if (emergency.getFlightId() != flightId) {
        throw ValidationException("Emergency " + emergencyId + " does not belong to flight " + flightId);
    }
    // 5. After landing, ground operations can begin.
    flightService_.transitionFlightStatus(flightId, FlightStatus::GROUND_OPERATIONS);
    emergency.updateStatus(EmergencyStatus::GROUND_HANDLING);
    db_.updateEmergency(emergency);
}

void EmergencyService::resolveEmergency(const std::string& emergencyId) {
    EmergencyLanding emergency = getEmergency(emergencyId);
    emergency.updateStatus(EmergencyStatus::RESOLVED);
    db_.updateEmergency(emergency);
}

EmergencyLanding EmergencyService::getEmergency(const std::string& emergencyId) const {
    for (auto& e : db_.listAllEmergencies()) {
        if (e.getEmergencyId() == emergencyId) return e;
    }
    throw NotFoundException("Emergency record not found: " + emergencyId);
}

} // namespace services
} // namespace agoms
