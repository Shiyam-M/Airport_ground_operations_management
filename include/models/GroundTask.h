#ifndef AGOMS_GROUND_TASK_H
#define AGOMS_GROUND_TASK_H

#include <string>
#include "models/Enums.h"
#include "utils/DateTime.h"

namespace agoms {

// A single ground-operations task (baggage handling, cleaning, fueling,
// catering, boarding support) tied to a flight and an assigned staff
// resource.
class GroundTask {
public:
    GroundTask(std::string taskId, std::string flightId, TaskType type,
               std::string assignedStaffId, util::DateTime startTime, util::DateTime endTime);

    const std::string& getTaskId() const { return taskId_; }
    const std::string& getFlightId() const { return flightId_; }
    TaskType getType() const { return type_; }
    const std::string& getAssignedStaffId() const { return assignedStaffId_; }
    TaskStatus getStatus() const { return status_; }
    util::DateTime getStartTime() const { return startTime_; }
    util::DateTime getEndTime() const { return endTime_; }

    void setAssignedStaffId(const std::string& id) { assignedStaffId_ = id; status_ = TaskStatus::ASSIGNED; }
    void updateStatus(TaskStatus status) { status_ = status; }

private:
    std::string taskId_;
    std::string flightId_;
    TaskType type_;
    std::string assignedStaffId_;
    TaskStatus status_;
    util::DateTime startTime_;
    util::DateTime endTime_;
};

} // namespace agoms

#endif // AGOMS_GROUND_TASK_H
