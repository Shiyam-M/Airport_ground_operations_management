#ifndef AGOMS_CONFLICT_RESULT_H
#define AGOMS_CONFLICT_RESULT_H

#include <string>
#include "models/Enums.h"

namespace agoms {

// Outcome of a ConflictDetector check.
class ConflictResult {
public:
    ConflictResult() : conflictType_(ConflictType::NONE) {}
    ConflictResult(ConflictType type, std::string details)
        : conflictType_(type), details_(std::move(details)) {}

    bool hasConflict() const { return conflictType_ != ConflictType::NONE; }
    ConflictType getConflictType() const { return conflictType_; }
    const std::string& getDetails() const { return details_; }

    static ConflictResult none() { return ConflictResult(); }

private:
    ConflictType conflictType_;
    std::string details_;
};

} // namespace agoms

#endif // AGOMS_CONFLICT_RESULT_H
