#ifndef AGOMS_ATC_SERVICE_H
#define AGOMS_ATC_SERVICE_H

#include <string>
#include "models/ATCStatus.h"
#include "repositories/AirportDatabase.h"

namespace agoms {
namespace services {

// Simulated Air Traffic Control integration. This is an academic mock: it
// derives plausible ATC information deterministically from the flight's
// current state rather than calling a real external ATC system, but the
// public interface (getFlightStatus/getATCClearance/getDepartureStatus) is
// exactly what a real integration would expose, so swapping this
// implementation out for a real ATC feed would not require changing any
// caller (FlightMonitor, DelayService, EmergencyService, ConsoleUI).
class ATCService {
public:
    explicit ATCService(repo::AirportDatabase& db) : db_(db) {}

    ATCStatus getFlightStatus(const std::string& flightId);
    ATCClearance getATCClearance(const std::string& flightId);
    std::string getDepartureStatus(const std::string& flightId);
    std::string getArrivalStatus(const std::string& flightId);

private:
    repo::AirportDatabase& db_;
};

} // namespace services
} // namespace agoms

#endif // AGOMS_ATC_SERVICE_H
