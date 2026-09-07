#include "services/WeatherService.h"
#include <random>
#include <chrono>

namespace agoms {
namespace services {

WeatherData WeatherService::getCurrentWeather() {
    static std::mt19937 rng(static_cast<unsigned>(
        std::chrono::system_clock::now().time_since_epoch().count()));

    std::uniform_real_distribution<double> tempDist(18.0, 40.0);
    std::uniform_real_distribution<double> windDist(0.0, 60.0);
    std::uniform_real_distribution<double> visDist(0.5, 10.0);
    std::uniform_real_distribution<double> precipDist(0.0, 30.0);
    std::uniform_int_distribution<int> condDist(0, 5);

    WeatherData data;
    data.temperatureCelsius = tempDist(rng);
    data.windSpeedKmh = windDist(rng);
    data.visibilityKm = visDist(rng);
    data.precipitationMm = precipDist(rng);
    data.condition = static_cast<WeatherCondition>(condDist(rng));
    data.observedAt = util::DateTime::now();

    db_.logWeather(data);
    return data;
}

WeatherAlert WeatherService::checkWeatherAlert(const WeatherData& data) const {
    WeatherAlert alert;

    if (data.condition == WeatherCondition::STORM || data.windSpeedKmh >= 50.0 ||
        data.visibilityKm < 1.0) {
        alert.level = WeatherAlertLevel::SEVERE;
        alert.message = "Severe conditions: high wind/storm or very low visibility. "
                         "Ground operations and departures may need to be suspended.";
    } else if (data.condition == WeatherCondition::FOG || data.windSpeedKmh >= 35.0 ||
               data.visibilityKm < 3.0 || data.precipitationMm >= 20.0) {
        alert.level = WeatherAlertLevel::WARNING;
        alert.message = "Degraded conditions (fog/high wind/heavy precipitation). "
                         "Expect possible delays.";
    } else if (data.windSpeedKmh >= 20.0 || data.precipitationMm >= 10.0) {
        alert.level = WeatherAlertLevel::ADVISORY;
        alert.message = "Minor weather advisory; monitor conditions.";
    } else {
        alert.level = WeatherAlertLevel::NONE;
        alert.message = "Conditions normal.";
    }
    return alert;
}

} // namespace services
} // namespace agoms
