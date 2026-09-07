#ifndef AGOMS_CONFLICT_DETECTOR_H
#define AGOMS_CONFLICT_DETECTOR_H

#include "models/AllocationRequest.h"
#include "models/ConflictResult.h"
#include "repositories/AirportDatabase.h"

namespace agoms {
namespace services {

// Central conflict-checking authority used by AllocationService before any
// resource is committed to an allocation. Consults AirportDatabase for
// current resource status and existing active allocations, and detects:
//   - resource unavailable (MAINTENANCE / already OCCUPIED)
//   - overlapping allocation time windows for the same resource
//   - invalid flight/resource combinations (unknown flight or resource ids)
// Returns a ConflictResult; ConflictResult::hasConflict() == true means the
// allocation MUST NOT proceed.
class ConflictDetector {
public:
    explicit ConflictDetector(repo::AirportDatabase& db) : db_(db) {}

    ConflictResult checkConflict(const AllocationRequest& request) const;

private:
    repo::AirportDatabase& db_;

    ConflictResult checkGate(const AllocationRequest& request) const;
    ConflictResult checkVehicle(const AllocationRequest& request) const;
    ConflictResult checkGroundStaff(const AllocationRequest& request) const;
};

} // namespace services
} // namespace agoms

#endif // AGOMS_CONFLICT_DETECTOR_H
