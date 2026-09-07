#include "services/FlightService.h"
#include "utils/Exceptions.h"

namespace agoms {
namespace services {

void FlightService::createFlight(const std::string& flightId, const std::string& flightNumber,
                                  const std::string& origin, const std::string& destination,
                                  const std::string& aircraftType, util::DateTime scheduledArrival,
                                  util::DateTime scheduledDeparture) {
    if (db_.findFlightById(flightId).has_value()) {
        throw ValidationException("Flight ID already exists: " + flightId);
    }
    Flight flight(flightId, flightNumber, origin, destination, aircraftType);
    db_.saveFlight(flight);

    std::string scheduleId = scheduleIdGen_.next();
    FlightSchedule schedule(scheduleId, flightId, scheduledArrival, scheduledDeparture);
    db_.saveFlightSchedule(schedule);
}

Flight FlightService::getFlight(const std::string& flightId) const {
    auto flight = db_.findFlightById(flightId);
    if (!flight.has_value()) {
        throw NotFoundException("Flight not found: " + flightId);
    }
    return flight.value();
}

std::vector<Flight> FlightService::listFlights() const {
    return db_.listFlights();
}

std::optional<FlightSchedule> FlightService::getSchedule(const std::string& flightId) const {
    return db_.findScheduleByFlightId(flightId);
}

void FlightService::transitionFlightStatus(const std::string& flightId, FlightStatus newStatus) {
    Flight flight = getFlight(flightId);
    flight.transitionTo(newStatus); // throws InvalidStateTransitionException if illegal
    db_.updateFlightStatus(flightId, newStatus);
}

} // namespace services
} // namespace agoms
