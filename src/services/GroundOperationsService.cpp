#include "services/GroundOperationsService.h"
#include "utils/Exceptions.h"

namespace agoms {
namespace services {

GroundTask GroundOperationsService::createTask(const std::string& flightId, TaskType type,
                                                const std::string& assignedStaffId,
                                                util::DateTime start, util::DateTime end) {
    if (!db_.findFlightById(flightId).has_value()) {
        throw NotFoundException("Cannot create ground task; unknown flight: " + flightId);
    }
    std::string taskId = taskIdGen_.next();
    GroundTask task(taskId, flightId, type, assignedStaffId, start, end);
    db_.saveGroundTask(task);
    return task;
}

void GroundOperationsService::updateTaskStatus(const std::string& taskId, TaskStatus status) {
    db_.updateTaskStatus(taskId, status);
}

bool GroundOperationsService::isGroundOperationComplete(const std::string& flightId) const {
    auto tasks = db_.listTasksForFlight(flightId);
    if (tasks.empty()) return false;
    for (const auto& t : tasks) {
        if (t.getStatus() != TaskStatus::COMPLETED) return false;
    }
    return true;
}

} // namespace services
} // namespace agoms
