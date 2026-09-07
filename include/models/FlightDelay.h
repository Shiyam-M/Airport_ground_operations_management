#ifndef AGOMS_FLIGHT_DELAY_H
#define AGOMS_FLIGHT_DELAY_H

#include <string>
#include "models/Enums.h"
#include "utils/DateTime.h"

namespace agoms {

// Record of a single delay event applied to a flight.
class FlightDelay {
public:
    FlightDelay(std::string delayId, std::string flightId, DelayReason reason,
                int delayMinutes, std::string notes = "");

    const std::string& getDelayId() const { return delayId_; }
    const std::string& getFlightId() const { return flightId_; }
    DelayReason getReason() const { return reason_; }
    int getDelayMinutes() const { return delayMinutes_; }
    const std::string& getNotes() const { return notes_; }
    util::DateTime getRecordedAt() const { return recordedAt_; }

private:
    std::string delayId_;
    std::string flightId_;
    DelayReason reason_;
    int delayMinutes_;
    std::string notes_;
    util::DateTime recordedAt_;
};

} // namespace agoms

#endif // AGOMS_FLIGHT_DELAY_H
