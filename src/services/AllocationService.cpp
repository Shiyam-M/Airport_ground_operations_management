#include "services/AllocationService.h"
#include "utils/Exceptions.h"
#include "models/Enums.h"

namespace agoms {
namespace services {

AllocationResult AllocationService::allocate(const AllocationRequest& request) {
    // Step 1-2: Validate the flight & its status.
    auto flightOpt = db_.findFlightById(request.flightId);
    if (!flightOpt.has_value()) {
        return AllocationResult::failure(
            ConflictResult(ConflictType::INVALID_RESOURCE, "Unknown flight ID: " + request.flightId));
    }
    Flight flight = flightOpt.value();
    static const FlightStatus allocatableStates[] = {
        FlightStatus::SCHEDULED, FlightStatus::APPROACHING, FlightStatus::ARRIVED,
        FlightStatus::EMERGENCY_LANDING, FlightStatus::GROUND_OPERATIONS, FlightStatus::DELAYED
    };
    bool statusOk = false;
    for (auto s : allocatableStates) if (s == flight.getStatus()) { statusOk = true; break; }
    if (!statusOk) {
        return AllocationResult::failure(ConflictResult(ConflictType::INVALID_FLIGHT_STATE,
            "Flight " + request.flightId + " is in state " + toString(flight.getStatus()) +
            " and cannot receive new resource allocations."));
    }

    // Steps 3-6: Gate / Vehicle / Ground staff availability + time-conflict checks.
    ConflictResult conflict = conflictDetector_.checkConflict(request);
    if (conflict.hasConflict()) {
        // If no conflict exists -> allocate; otherwise: do NOT allocate, return FAILED.
        return AllocationResult::failure(conflict);
    }

    // Step 7: No conflict -> allocate resources.
    std::string allocationId = allocationIdGen_.next();
    Allocation allocation(allocationId, request.flightId, request.gateId, request.vehicleId,
                           request.groundStaffIds, request.windowStart, request.windowEnd);

    // Step 8: Update resource status (AVAILABLE -> OCCUPIED).
    db_.updateGateStatus(request.gateId, ResourceStatus::OCCUPIED);
    db_.updateVehicleStatus(request.vehicleId, ResourceStatus::OCCUPIED);
    for (const auto& staffId : request.groundStaffIds) {
        db_.updateGroundStaffResourceStatus(staffId, ResourceStatus::OCCUPIED);
    }

    // Step 9: Store allocation.
    db_.saveAllocation(allocation);

    // Step 10: Return successful AllocationResult.
    return AllocationResult::success(allocationId);
}

void AllocationService::release(const std::string& allocationId) {
    auto allocOpt = db_.findAllocationById(allocationId);
    if (!allocOpt.has_value()) {
        throw NotFoundException("Allocation not found: " + allocationId);
    }
    Allocation allocation = allocOpt.value();
    if (allocation.getStatus() == AllocationStatus::RELEASED) {
        throw ValidationException("Allocation " + allocationId + " is already released.");
    }

    db_.updateGateStatus(allocation.getGateId(), ResourceStatus::AVAILABLE);
    db_.updateVehicleStatus(allocation.getVehicleId(), ResourceStatus::AVAILABLE);
    for (const auto& staffId : allocation.getGroundStaffIds()) {
        db_.updateGroundStaffResourceStatus(staffId, ResourceStatus::AVAILABLE);
    }
    db_.updateAllocationStatus(allocationId, AllocationStatus::RELEASED);
}

} // namespace services
} // namespace agoms
