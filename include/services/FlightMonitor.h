#ifndef AGOMS_FLIGHT_MONITOR_H
#define AGOMS_FLIGHT_MONITOR_H

#include <string>
#include <vector>
#include "services/ATCService.h"
#include "services/WeatherService.h"
#include "services/FlightService.h"

namespace agoms {
namespace services {

// Result of a single monitoring pass over one flight.
struct MonitoringSnapshot {
    std::string flightId;
    FlightStatus flightStatus;
    ATCStatus atcStatus;
    WeatherData weather;
    WeatherAlert weatherAlert;
    std::vector<std::string> issues; // human-readable operational issues detected
};

// FlightMonitor composes ATCService + WeatherService + FlightService to
// watch a flight's operational picture, per the UML composition:
//   FlightMonitor --- ATCService
//                 --- WeatherService
//                 --- Flight (via FlightService)
// It does not itself decide to delay a flight (that responsibility belongs
// to DelayService) — it only observes and reports issues that a human
// Operations Manager (or DelayService) should act on.
class FlightMonitor {
public:
    FlightMonitor(repo::AirportDatabase& db, ATCService& atc, WeatherService& weather, FlightService& flights)
        : db_(db), atc_(atc), weather_(weather), flights_(flights) {}

    MonitoringSnapshot monitor(const std::string& flightId);

private:
    repo::AirportDatabase& db_;
    ATCService& atc_;
    WeatherService& weather_;
    FlightService& flights_;
};

} // namespace services
} // namespace agoms

#endif // AGOMS_FLIGHT_MONITOR_H
