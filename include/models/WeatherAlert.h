#ifndef AGOMS_WEATHER_ALERT_H
#define AGOMS_WEATHER_ALERT_H

#include <string>
#include "models/Enums.h"

namespace agoms {

// Derived operational alert produced from a WeatherData snapshot.
struct WeatherAlert {
    WeatherAlertLevel level = WeatherAlertLevel::NONE;
    std::string message;

    bool requiresAttention() const {
        return level == WeatherAlertLevel::WARNING || level == WeatherAlertLevel::SEVERE;
    }
};

} // namespace agoms

#endif // AGOMS_WEATHER_ALERT_H
