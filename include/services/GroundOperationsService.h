#ifndef AGOMS_GROUND_OPERATIONS_SERVICE_H
#define AGOMS_GROUND_OPERATIONS_SERVICE_H

#include <string>
#include <vector>
#include "models/GroundTask.h"
#include "repositories/AirportDatabase.h"
#include "utils/DateTime.h"

namespace agoms {
namespace services {

// Manages GroundOperations: creation and status tracking of GroundTasks
// (baggage handling, aircraft cleaning, fueling, catering, boarding
// support) for a flight. Ground Staff users update task status through
// this service; Operations Managers create/assign tasks and generate the
// overall ground-operations schedule for a flight.
class GroundOperationsService {
public:
    explicit GroundOperationsService(repo::AirportDatabase& db) : db_(db) {}

    GroundTask createTask(const std::string& flightId, TaskType type, const std::string& assignedStaffId,
                           util::DateTime start, util::DateTime end);

    void updateTaskStatus(const std::string& taskId, TaskStatus status);

    std::vector<GroundTask> listTasksForFlight(const std::string& flightId) const {
        return db_.listTasksForFlight(flightId);
    }
    std::vector<GroundTask> listTasksForStaff(const std::string& staffId) const {
        return db_.listTasksForStaff(staffId);
    }
    std::vector<GroundTask> listAllTasks() const { return db_.listAllTasks(); }

    // Returns true once every task for the given flight has status COMPLETED.
    bool isGroundOperationComplete(const std::string& flightId) const;

private:
    repo::AirportDatabase& db_;
    util::IdGenerator taskIdGen_{"GT", 1};
};

} // namespace services
} // namespace agoms

#endif // AGOMS_GROUND_OPERATIONS_SERVICE_H
