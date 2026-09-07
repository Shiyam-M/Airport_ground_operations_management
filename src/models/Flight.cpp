#include "models/Flight.h"
#include "utils/Exceptions.h"
#include <vector>
#include <utility>

namespace agoms {

Flight::Flight(std::string flightId, std::string flightNumber, std::string origin,
               std::string destination, std::string aircraftType)
    : flightId_(std::move(flightId)),
      flightNumber_(std::move(flightNumber)),
      origin_(std::move(origin)),
      destination_(std::move(destination)),
      aircraftType_(std::move(aircraftType)),
      status_(FlightStatus::SCHEDULED) {}

bool Flight::isTransitionAllowed(FlightStatus from, FlightStatus to) {
    using FS = FlightStatus;
    // Adjacency list mirrors the state diagram in section 6 of the spec /
    // docs/design.md. Kept as a static table rather than a full State
    // Pattern class hierarchy: the rules are simple enough that a table is
    // easier to read, audit and unit-test than 11 separate State classes.
    static const std::vector<std::pair<FS, FS>> allowed = {
        {FS::SCHEDULED, FS::APPROACHING},
        {FS::SCHEDULED, FS::CANCELLED},
        {FS::APPROACHING, FS::ARRIVED},
        {FS::APPROACHING, FS::EMERGENCY_LANDING},
        {FS::APPROACHING, FS::CANCELLED},
        {FS::EMERGENCY_LANDING, FS::GROUND_OPERATIONS},
        {FS::ARRIVED, FS::GROUND_OPERATIONS},
        {FS::GROUND_OPERATIONS, FS::BOARDING},
        {FS::GROUND_OPERATIONS, FS::DELAYED},
        {FS::BOARDING, FS::READY_FOR_DEPARTURE},
        {FS::BOARDING, FS::DELAYED},
        {FS::READY_FOR_DEPARTURE, FS::DEPARTED},
        {FS::READY_FOR_DEPARTURE, FS::DELAYED},
        {FS::DEPARTED, FS::COMPLETED},
        // DELAYED resumes into whichever operational state it was delayed
        // from; DelayService decides the specific target, so all forward
        // operational states plus CANCELLED are reachable from DELAYED.
        {FS::DELAYED, FS::GROUND_OPERATIONS},
        {FS::DELAYED, FS::BOARDING},
        {FS::DELAYED, FS::READY_FOR_DEPARTURE},
        {FS::DELAYED, FS::CANCELLED},
    };
    if (from == to) return false;
    for (const auto& edge : allowed) {
        if (edge.first == from && edge.second == to) return true;
    }
    return false;
}

void Flight::transitionTo(FlightStatus newStatus) {
    if (!isTransitionAllowed(status_, newStatus)) {
        throw InvalidStateTransitionException(
            "Illegal flight state transition for " + flightId_ + ": " +
            toString(status_) + " -> " + toString(newStatus));
    }
    status_ = newStatus;
}

} // namespace agoms
