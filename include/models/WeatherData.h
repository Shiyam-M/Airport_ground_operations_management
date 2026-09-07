#ifndef AGOMS_WEATHER_DATA_H
#define AGOMS_WEATHER_DATA_H

#include <string>
#include "models/Enums.h"
#include "utils/DateTime.h"

namespace agoms {

// Snapshot of current airport weather conditions.
struct WeatherData {
    double temperatureCelsius = 0.0;
    double windSpeedKmh = 0.0;
    double visibilityKm = 0.0;
    double precipitationMm = 0.0;
    WeatherCondition condition = WeatherCondition::CLEAR;
    util::DateTime observedAt;
};

} // namespace agoms

#endif // AGOMS_WEATHER_DATA_H
