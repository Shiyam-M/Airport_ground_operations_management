#ifndef AGOMS_ALLOCATION_REQUEST_H
#define AGOMS_ALLOCATION_REQUEST_H

#include <string>
#include <vector>
#include "utils/DateTime.h"

namespace agoms {

// Plain data carrier describing what the caller wants allocated to a
// flight, for a given time window. Passed from the UI layer down through
// AllocationService -> ConflictDetector -> AirportDatabase.
struct AllocationRequest {
    std::string flightId;
    std::string gateId;
    std::string vehicleId;
    std::vector<std::string> groundStaffIds;
    util::DateTime windowStart;
    util::DateTime windowEnd;
};

} // namespace agoms

#endif // AGOMS_ALLOCATION_REQUEST_H
