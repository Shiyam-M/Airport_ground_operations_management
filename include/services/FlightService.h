#ifndef AGOMS_FLIGHT_SERVICE_H
#define AGOMS_FLIGHT_SERVICE_H

#include <string>
#include <vector>
#include <optional>
#include "models/Flight.h"
#include "models/FlightSchedule.h"
#include "repositories/AirportDatabase.h"
#include "utils/DateTime.h"

namespace agoms {
namespace services {

// Business logic for flight master data + lifecycle transitions.
// Administrator uses createFlight(); Operations Manager / FlightMonitor /
// DelayService / EmergencyService use transitionFlightStatus().
class FlightService {
public:
    explicit FlightService(repo::AirportDatabase& db) : db_(db) {}

    void createFlight(const std::string& flightId, const std::string& flightNumber,
                       const std::string& origin, const std::string& destination,
                       const std::string& aircraftType, util::DateTime scheduledArrival,
                       util::DateTime scheduledDeparture);

    Flight getFlight(const std::string& flightId) const; // throws NotFoundException
    std::vector<Flight> listFlights() const;
    std::optional<FlightSchedule> getSchedule(const std::string& flightId) const;

    // Validates and persists a flight-state transition; throws
    // InvalidStateTransitionException on illegal transitions.
    void transitionFlightStatus(const std::string& flightId, FlightStatus newStatus);

private:
    repo::AirportDatabase& db_;
    util::IdGenerator scheduleIdGen_{"SC", 1000};
};

} // namespace services
} // namespace agoms

#endif // AGOMS_FLIGHT_SERVICE_H
