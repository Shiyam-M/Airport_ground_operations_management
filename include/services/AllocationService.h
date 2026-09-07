#ifndef AGOMS_ALLOCATION_SERVICE_H
#define AGOMS_ALLOCATION_SERVICE_H

#include <string>
#include <vector>
#include "models/AllocationRequest.h"
#include "models/AllocationResult.h"
#include "models/Allocation.h"
#include "repositories/AirportDatabase.h"
#include "services/ConflictDetector.h"
#include "utils/DateTime.h"

namespace agoms {
namespace services {

// Orchestrates the full resource-allocation workflow described in the
// project specification:
//   UI -> AllocationService -> (validate flight) -> ConflictDetector
//      -> AirportDatabase -> AllocationResult
//
// On success: resources are marked OCCUPIED and an Allocation record is
// persisted. On conflict: nothing is mutated and a FAILED AllocationResult
// carrying the ConflictResult is returned.
class AllocationService {
public:
    explicit AllocationService(repo::AirportDatabase& db)
        : db_(db), conflictDetector_(db) {}

    AllocationResult allocate(const AllocationRequest& request);

    // Releases a successful allocation: resources return to AVAILABLE and
    // the allocation record is marked RELEASED. Typically called when
    // ground operations for a flight complete.
    void release(const std::string& allocationId);

    std::vector<Allocation> listAllocationsForFlight(const std::string& flightId) const {
        return db_.listAllocationsForFlight(flightId);
    }
    std::vector<Allocation> listAllAllocations() const { return db_.listAllAllocations(); }

private:
    repo::AirportDatabase& db_;
    ConflictDetector conflictDetector_;
    util::IdGenerator allocationIdGen_{"AL", 1};
};

} // namespace services
} // namespace agoms

#endif // AGOMS_ALLOCATION_SERVICE_H
