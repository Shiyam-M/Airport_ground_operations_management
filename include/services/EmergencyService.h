#ifndef AGOMS_EMERGENCY_SERVICE_H
#define AGOMS_EMERGENCY_SERVICE_H

#include <string>
#include <vector>
#include "models/EmergencyLanding.h"
#include "repositories/AirportDatabase.h"
#include "services/FlightService.h"

namespace agoms {
namespace services {

// Implements the emergency-landing workflow from the specification:
//   1. declareEmergency(): Flight -> EMERGENCY_LANDING, details recorded.
//   2. recordLanding(): landing time captured, emergency status -> LANDED.
//   3. beginGroundHandling(): Flight -> GROUND_OPERATIONS, emergency -> GROUND_HANDLING.
//   4. resolveEmergency(): emergency -> RESOLVED (informational close-out).
class EmergencyService {
public:
    EmergencyService(repo::AirportDatabase& db, FlightService& flightService)
        : db_(db), flightService_(flightService) {}

    EmergencyLanding declareEmergency(const std::string& flightId, const std::string& reason);
    EmergencyLanding recordLanding(const std::string& emergencyId);
    void beginGroundHandling(const std::string& flightId, const std::string& emergencyId);
    void resolveEmergency(const std::string& emergencyId);

    std::vector<EmergencyLanding> listAllEmergencies() const { return db_.listAllEmergencies(); }
    EmergencyLanding getEmergency(const std::string& emergencyId) const; // throws NotFoundException

private:
    repo::AirportDatabase& db_;
    FlightService& flightService_;
    util::IdGenerator emergencyIdGen_{"EM", 1};
};

} // namespace services
} // namespace agoms

#endif // AGOMS_EMERGENCY_SERVICE_H
