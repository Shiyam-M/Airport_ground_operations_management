#include "models/FlightSchedule.h"

namespace agoms {

FlightSchedule::FlightSchedule(std::string scheduleId, std::string flightId,
                                util::DateTime scheduledArrival, util::DateTime scheduledDeparture)
    : scheduleId_(std::move(scheduleId)),
      flightId_(std::move(flightId)),
      scheduledArrival_(scheduledArrival),
      scheduledDeparture_(scheduledDeparture),
      estimatedArrival_(scheduledArrival),
      estimatedDeparture_(scheduledDeparture) {}

void FlightSchedule::applyDelay(int delayMinutes) {
    estimatedArrival_ = estimatedArrival_.addMinutes(delayMinutes);
    estimatedDeparture_ = estimatedDeparture_.addMinutes(delayMinutes);
}

} // namespace agoms
