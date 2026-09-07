#include "models/GroundTask.h"

namespace agoms {

GroundTask::GroundTask(std::string taskId, std::string flightId, TaskType type,
                        std::string assignedStaffId, util::DateTime startTime, util::DateTime endTime)
    : taskId_(std::move(taskId)),
      flightId_(std::move(flightId)),
      type_(type),
      assignedStaffId_(std::move(assignedStaffId)),
      status_(assignedStaffId_.empty() ? TaskStatus::PENDING : TaskStatus::ASSIGNED),
      startTime_(startTime),
      endTime_(endTime) {}

} // namespace agoms
