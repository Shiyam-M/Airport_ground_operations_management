#ifndef AGOMS_FLIGHT_SCHEDULE_H
#define AGOMS_FLIGHT_SCHEDULE_H

#include <string>
#include "utils/DateTime.h"

namespace agoms {

// Scheduling information for a flight: scheduled/estimated arrival &
// departure windows. Kept as a distinct entity from Flight so that
// FlightDelay can update timings without mutating Flight's identity fields.
class FlightSchedule {
public:
    FlightSchedule(std::string scheduleId, std::string flightId,
                    util::DateTime scheduledArrival, util::DateTime scheduledDeparture);

    const std::string& getScheduleId() const { return scheduleId_; }
    const std::string& getFlightId() const { return flightId_; }

    util::DateTime getScheduledArrival() const { return scheduledArrival_; }
    util::DateTime getScheduledDeparture() const { return scheduledDeparture_; }
    util::DateTime getEstimatedArrival() const { return estimatedArrival_; }
    util::DateTime getEstimatedDeparture() const { return estimatedDeparture_; }

    void setEstimatedArrival(util::DateTime t) { estimatedArrival_ = t; }
    void setEstimatedDeparture(util::DateTime t) { estimatedDeparture_ = t; }

    // Shifts both estimated arrival & departure forward by delayMinutes.
    void applyDelay(int delayMinutes);

private:
    std::string scheduleId_;
    std::string flightId_;
    util::DateTime scheduledArrival_;
    util::DateTime scheduledDeparture_;
    util::DateTime estimatedArrival_;
    util::DateTime estimatedDeparture_;
};

} // namespace agoms

#endif // AGOMS_FLIGHT_SCHEDULE_H
