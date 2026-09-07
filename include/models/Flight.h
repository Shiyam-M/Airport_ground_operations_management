#ifndef AGOMS_FLIGHT_H
#define AGOMS_FLIGHT_H

#include <string>
#include "models/Enums.h"
#include "utils/DateTime.h"

namespace agoms {

// Core Flight entity: identity + current lifecycle status. FlightSchedule
// (models/FlightSchedule.h) holds the associated timing information
// separately, keeping Flight focused on identity/state and avoiding a
// bloated "god object".
class Flight {
public:
    Flight(std::string flightId, std::string flightNumber, std::string origin,
           std::string destination, std::string aircraftType);

    const std::string& getFlightId() const { return flightId_; }
    const std::string& getFlightNumber() const { return flightNumber_; }
    const std::string& getOrigin() const { return origin_; }
    const std::string& getDestination() const { return destination_; }
    const std::string& getAircraftType() const { return aircraftType_; }

    FlightStatus getStatus() const { return status_; }

    // Validates the transition against the flight-lifecycle state diagram
    // (docs/design.md) and throws InvalidStateTransitionException if illegal.
    void transitionTo(FlightStatus newStatus);

    // Used only when re-hydrating a Flight from the database, where the
    // stored status is already known-valid and should not be re-validated.
    void setStatusUnchecked(FlightStatus status) { status_ = status; }

    static bool isTransitionAllowed(FlightStatus from, FlightStatus to);

private:
    std::string flightId_;
    std::string flightNumber_;
    std::string origin_;
    std::string destination_;
    std::string aircraftType_;
    FlightStatus status_;
};

} // namespace agoms

#endif // AGOMS_FLIGHT_H
