#include "models/Allocation.h"

namespace agoms {

Allocation::Allocation(std::string allocationId, std::string flightId, std::string gateId,
                        std::string vehicleId, std::vector<std::string> groundStaffIds,
                        util::DateTime windowStart, util::DateTime windowEnd)
    : allocationId_(std::move(allocationId)),
      flightId_(std::move(flightId)),
      gateId_(std::move(gateId)),
      vehicleId_(std::move(vehicleId)),
      groundStaffIds_(std::move(groundStaffIds)),
      windowStart_(windowStart),
      windowEnd_(windowEnd),
      status_(AllocationStatus::SUCCESS) {}

} // namespace agoms
