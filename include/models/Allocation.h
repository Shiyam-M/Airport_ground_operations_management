#ifndef AGOMS_ALLOCATION_H
#define AGOMS_ALLOCATION_H

#include <string>
#include <vector>
#include "models/Enums.h"
#include "utils/DateTime.h"

namespace agoms {

// A persisted record that a specific gate/vehicle/staff set was allocated
// to a flight for a given time window. Created by AllocationService once a
// ConflictDetector check passes.
class Allocation {
public:
    Allocation(std::string allocationId, std::string flightId, std::string gateId,
               std::string vehicleId, std::vector<std::string> groundStaffIds,
               util::DateTime windowStart, util::DateTime windowEnd);

    const std::string& getAllocationId() const { return allocationId_; }
    const std::string& getFlightId() const { return flightId_; }
    const std::string& getGateId() const { return gateId_; }
    const std::string& getVehicleId() const { return vehicleId_; }
    const std::vector<std::string>& getGroundStaffIds() const { return groundStaffIds_; }
    util::DateTime getWindowStart() const { return windowStart_; }
    util::DateTime getWindowEnd() const { return windowEnd_; }
    AllocationStatus getStatus() const { return status_; }

    void release() { status_ = AllocationStatus::RELEASED; }

private:
    std::string allocationId_;
    std::string flightId_;
    std::string gateId_;
    std::string vehicleId_;
    std::vector<std::string> groundStaffIds_;
    util::DateTime windowStart_;
    util::DateTime windowEnd_;
    AllocationStatus status_;
};

} // namespace agoms

#endif // AGOMS_ALLOCATION_H
