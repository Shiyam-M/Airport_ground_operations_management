#include "services/ATCService.h"
#include "utils/Exceptions.h"

namespace agoms {
namespace services {

ATCStatus ATCService::getFlightStatus(const std::string& flightId) {
    auto flightOpt = db_.findFlightById(flightId);
    if (!flightOpt.has_value()) {
        throw NotFoundException("Cannot fetch ATC status; unknown flight: " + flightId);
    }
    Flight flight = flightOpt.value();

    ATCStatus status;
    status.flightId = flightId;
    status.clearance = getATCClearance(flightId);
    status.departureInfo = getDepartureStatus(flightId);
    status.arrivalInfo = getArrivalStatus(flightId);
    status.remarks = "Simulated ATC feed - flight state: " + toString(flight.getStatus());

    db_.logAtcStatus(status);
    return status;
}

ATCClearance ATCService::getATCClearance(const std::string& flightId) {
    auto flightOpt = db_.findFlightById(flightId);
    if (!flightOpt.has_value()) return ATCClearance::PENDING;
    switch (flightOpt->getStatus()) {
        case FlightStatus::SCHEDULED:
        case FlightStatus::APPROACHING:
            return ATCClearance::PENDING;
        case FlightStatus::EMERGENCY_LANDING:
            return ATCClearance::HOLD;
        case FlightStatus::ARRIVED:
        case FlightStatus::GROUND_OPERATIONS:
        case FlightStatus::BOARDING:
        case FlightStatus::READY_FOR_DEPARTURE:
        case FlightStatus::DEPARTED:
        case FlightStatus::COMPLETED:
            return ATCClearance::GRANTED;
        case FlightStatus::DELAYED:
            return ATCClearance::HOLD;
        case FlightStatus::CANCELLED:
            return ATCClearance::DENIED;
    }
    return ATCClearance::PENDING;
}

std::string ATCService::getDepartureStatus(const std::string& flightId) {
    auto flightOpt = db_.findFlightById(flightId);
    if (!flightOpt.has_value()) return "UNKNOWN";
    switch (flightOpt->getStatus()) {
        case FlightStatus::READY_FOR_DEPARTURE: return "Cleared for pushback/taxi";
        case FlightStatus::DEPARTED:             return "Airborne, en route";
        case FlightStatus::COMPLETED:            return "Flight completed";
        case FlightStatus::DELAYED:              return "Departure slot delayed";
        case FlightStatus::CANCELLED:            return "Departure cancelled";
        default:                                 return "Not yet ready for departure";
    }
}

std::string ATCService::getArrivalStatus(const std::string& flightId) {
    auto flightOpt = db_.findFlightById(flightId);
    if (!flightOpt.has_value()) return "UNKNOWN";
    switch (flightOpt->getStatus()) {
        case FlightStatus::APPROACHING:        return "On approach, awaiting runway clearance";
        case FlightStatus::ARRIVED:            return "Landed, taxiing to gate";
        case FlightStatus::EMERGENCY_LANDING:  return "Emergency approach in progress";
        case FlightStatus::SCHEDULED:          return "Not yet airborne";
        default:                               return "Arrival complete";
    }
}

} // namespace services
} // namespace agoms
