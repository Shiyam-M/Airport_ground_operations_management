#include "services/DelayService.h"
#include "utils/Exceptions.h"

namespace agoms {
namespace services {

DelayOutcome DelayService::applyDelay(const std::string& flightId, DelayReason reason,
                                       int delayMinutes, const std::string& notes) {
    if (delayMinutes <= 0) {
        throw ValidationException("Delay duration must be a positive number of minutes.");
    }

    Flight flight = flightService_.getFlight(flightId); // throws NotFoundException

    // 1-2. Record delay reason & duration.
    std::string delayId = delayIdGen_.next();
    FlightDelay delay(delayId, flightId, reason, delayMinutes, notes);
    db_.saveDelay(delay);

    // 3. Update flight status to DELAYED (only legal from an operational
    //    ground state per the flight lifecycle state diagram).
    flightService_.transitionFlightStatus(flightId, FlightStatus::DELAYED);

    // 4. Update affected schedule.
    auto scheduleOpt = db_.findScheduleByFlightId(flightId);
    if (!scheduleOpt.has_value()) {
        throw NotFoundException("No schedule found for flight: " + flightId);
    }
    FlightSchedule schedule = scheduleOpt.value();
    schedule.applyDelay(delayMinutes);
    db_.updateScheduleEstimates(flightId, schedule.getEstimatedArrival(), schedule.getEstimatedDeparture());

    // 5-6. Check whether allocated resources are still conflict-free for
    //      the shifted time window; warn (rather than silently
    //      auto-reassign a different physical resource) if not.
    DelayOutcome outcome{delay, schedule, {}};
    auto allocations = db_.listAllocationsForFlight(flightId);
    for (const auto& alloc : allocations) {
        if (alloc.getStatus() != AllocationStatus::SUCCESS) continue;

        AllocationRequest probe;
        probe.flightId = flightId;
        probe.gateId = alloc.getGateId();
        probe.vehicleId = alloc.getVehicleId();
        probe.groundStaffIds = alloc.getGroundStaffIds();
        probe.windowStart = schedule.getEstimatedArrival();
        probe.windowEnd = schedule.getEstimatedDeparture();

        ConflictResult conflict = conflictDetector_.checkConflict(probe);
        if (conflict.hasConflict()) {
            outcome.resourceWarnings.push_back(
                "Allocation " + alloc.getAllocationId() + " may need reallocation: " + conflict.getDetails());
        }
    }

    // 7. Notify/display the updated schedule is left to the UI layer, which
    //    renders `outcome` after this call returns.
    return outcome;
}

} // namespace services
} // namespace agoms
