#include "models/FlightDelay.h"

namespace agoms {

FlightDelay::FlightDelay(std::string delayId, std::string flightId, DelayReason reason,
                          int delayMinutes, std::string notes)
    : delayId_(std::move(delayId)),
      flightId_(std::move(flightId)),
      reason_(reason),
      delayMinutes_(delayMinutes),
      notes_(std::move(notes)),
      recordedAt_(util::DateTime::now()) {}

} // namespace agoms
