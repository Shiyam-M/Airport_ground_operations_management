#include "services/FlightMonitor.h"

namespace agoms {
namespace services {

MonitoringSnapshot FlightMonitor::monitor(const std::string& flightId) {
    MonitoringSnapshot snapshot;
    snapshot.flightId = flightId;

    Flight flight = flights_.getFlight(flightId); // throws NotFoundException if unknown
    snapshot.flightStatus = flight.getStatus();

    snapshot.atcStatus = atc_.getFlightStatus(flightId);
    snapshot.weather = weather_.getCurrentWeather();
    snapshot.weatherAlert = weather_.checkWeatherAlert(snapshot.weather);

    // Detect operational issues from the combined ATC + weather picture.
    if (snapshot.weatherAlert.requiresAttention()) {
        snapshot.issues.push_back("Weather: " + snapshot.weatherAlert.message);
    }
    if (snapshot.atcStatus.clearance == ATCClearance::HOLD) {
        snapshot.issues.push_back("ATC has placed the flight on HOLD.");
    }
    if (snapshot.atcStatus.clearance == ATCClearance::DENIED) {
        snapshot.issues.push_back("ATC clearance DENIED.");
    }
    if (flight.getStatus() == FlightStatus::EMERGENCY_LANDING) {
        snapshot.issues.push_back("Flight is in an EMERGENCY_LANDING state.");
    }

    return snapshot;
}

} // namespace services
} // namespace agoms
