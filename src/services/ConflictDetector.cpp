#include "services/ConflictDetector.h"

namespace agoms {
namespace services {

ConflictResult ConflictDetector::checkConflict(const AllocationRequest& request) const {
    // 1. Validate flight & resource existence (invalid flight/resource combination).
    if (!db_.findFlightById(request.flightId).has_value()) {
        return ConflictResult(ConflictType::INVALID_RESOURCE,
                               "Unknown flight ID: " + request.flightId);
    }
    if (!db_.findGate(request.gateId).has_value()) {
        return ConflictResult(ConflictType::INVALID_RESOURCE, "Unknown gate ID: " + request.gateId);
    }
    if (!db_.findVehicle(request.vehicleId).has_value()) {
        return ConflictResult(ConflictType::INVALID_RESOURCE, "Unknown vehicle ID: " + request.vehicleId);
    }
    for (const auto& staffId : request.groundStaffIds) {
        if (!db_.findGroundStaffResource(staffId).has_value()) {
            return ConflictResult(ConflictType::INVALID_RESOURCE,
                                   "Unknown ground staff resource ID: " + staffId);
        }
    }

    // 2. Per-resource availability + time-overlap checks.
    ConflictResult gateResult = checkGate(request);
    if (gateResult.hasConflict()) return gateResult;

    ConflictResult vehicleResult = checkVehicle(request);
    if (vehicleResult.hasConflict()) return vehicleResult;

    ConflictResult staffResult = checkGroundStaff(request);
    if (staffResult.hasConflict()) return staffResult;

    return ConflictResult::none();
}

ConflictResult ConflictDetector::checkGate(const AllocationRequest& request) const {
    Gate gate = db_.findGate(request.gateId).value();
    if (gate.getStatus() == ResourceStatus::MAINTENANCE) {
        return ConflictResult(ConflictType::GATE_UNAVAILABLE,
                               "Gate " + request.gateId + " is under MAINTENANCE.");
    }
    auto activeAllocations = db_.listActiveAllocationsForGate(request.gateId);
    for (const auto& alloc : activeAllocations) {
        if (alloc.getFlightId() == request.flightId) continue; // re-allocating same flight is fine
        if (util::timeWindowsOverlap(request.windowStart, request.windowEnd,
                                      alloc.getWindowStart(), alloc.getWindowEnd())) {
            return ConflictResult(ConflictType::GATE_UNAVAILABLE,
                "Gate " + request.gateId + " is already allocated to " + alloc.getFlightId() +
                " for " + alloc.getWindowStart().toString() + " - " + alloc.getWindowEnd().toString() + ".");
        }
    }
    return ConflictResult::none();
}

ConflictResult ConflictDetector::checkVehicle(const AllocationRequest& request) const {
    Vehicle vehicle = db_.findVehicle(request.vehicleId).value();
    if (vehicle.getStatus() == ResourceStatus::MAINTENANCE) {
        return ConflictResult(ConflictType::VEHICLE_UNAVAILABLE,
                               "Vehicle " + request.vehicleId + " is under MAINTENANCE.");
    }
    auto activeAllocations = db_.listActiveAllocationsForVehicle(request.vehicleId);
    for (const auto& alloc : activeAllocations) {
        if (alloc.getFlightId() == request.flightId) continue;
        if (util::timeWindowsOverlap(request.windowStart, request.windowEnd,
                                      alloc.getWindowStart(), alloc.getWindowEnd())) {
            return ConflictResult(ConflictType::VEHICLE_UNAVAILABLE,
                "Vehicle " + request.vehicleId + " is already allocated to " + alloc.getFlightId() +
                " for " + alloc.getWindowStart().toString() + " - " + alloc.getWindowEnd().toString() + ".");
        }
    }
    return ConflictResult::none();
}

ConflictResult ConflictDetector::checkGroundStaff(const AllocationRequest& request) const {
    for (const auto& staffId : request.groundStaffIds) {
        GroundStaffResource staff = db_.findGroundStaffResource(staffId).value();
        if (staff.getStatus() == ResourceStatus::MAINTENANCE) {
            return ConflictResult(ConflictType::GROUND_STAFF_UNAVAILABLE,
                                   "Ground staff " + staffId + " is unavailable (MAINTENANCE/leave).");
        }
        auto activeAllocations = db_.listActiveAllocationsForStaff(staffId);
        for (const auto& alloc : activeAllocations) {
            if (alloc.getFlightId() == request.flightId) continue;
            if (util::timeWindowsOverlap(request.windowStart, request.windowEnd,
                                          alloc.getWindowStart(), alloc.getWindowEnd())) {
                return ConflictResult(ConflictType::GROUND_STAFF_UNAVAILABLE,
                    "Ground staff " + staffId + " is already allocated to " + alloc.getFlightId() +
                    " for " + alloc.getWindowStart().toString() + " - " + alloc.getWindowEnd().toString() + ".");
            }
        }
    }
    return ConflictResult::none();
}

} // namespace services
} // namespace agoms
