#ifndef AGOMS_ALLOCATION_RESULT_H
#define AGOMS_ALLOCATION_RESULT_H

#include <string>
#include "models/Enums.h"
#include "models/ConflictResult.h"

namespace agoms {

// Outcome of an allocation attempt, returned to the caller (UI layer).
class AllocationResult {
public:
    static AllocationResult success(std::string allocationId) {
        AllocationResult r;
        r.status_ = AllocationStatus::SUCCESS;
        r.allocationId_ = std::move(allocationId);
        return r;
    }
    static AllocationResult failure(ConflictResult conflict) {
        AllocationResult r;
        r.status_ = AllocationStatus::FAILED;
        r.conflict_ = std::move(conflict);
        return r;
    }

    bool isSuccess() const { return status_ == AllocationStatus::SUCCESS; }
    AllocationStatus getStatus() const { return status_; }
    const std::string& getAllocationId() const { return allocationId_; }
    const ConflictResult& getConflict() const { return conflict_; }

private:
    AllocationStatus status_ = AllocationStatus::FAILED;
    std::string allocationId_;
    ConflictResult conflict_;
};

} // namespace agoms

#endif // AGOMS_ALLOCATION_RESULT_H
