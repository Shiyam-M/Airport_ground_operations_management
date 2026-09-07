#ifndef AGOMS_WEATHER_SERVICE_H
#define AGOMS_WEATHER_SERVICE_H

#include "models/WeatherData.h"
#include "models/WeatherAlert.h"
#include "repositories/AirportDatabase.h"

namespace agoms {
namespace services {

// Simulated Weather integration (academic mock; no real external API call).
// Produces a pseudo-random-but-bounded weather snapshot each time
// getCurrentWeather() is called, seeded from the system clock, and derives
// an operational WeatherAlert from it via checkWeatherAlert().
class WeatherService {
public:
    explicit WeatherService(repo::AirportDatabase& db) : db_(db) {}

    WeatherData getCurrentWeather();
    WeatherAlert checkWeatherAlert(const WeatherData& data) const;

private:
    repo::AirportDatabase& db_;
};

} // namespace services
} // namespace agoms

#endif // AGOMS_WEATHER_SERVICE_H
