#ifndef AGOMS_DELAY_SERVICE_H
#define AGOMS_DELAY_SERVICE_H

#include <string>
#include <vector>
#include "models/FlightDelay.h"
#include "models/FlightSchedule.h"
#include "models/Enums.h"
#include "repositories/AirportDatabase.h"
#include "services/FlightService.h"
#include "services/AllocationService.h"
#include "services/ConflictDetector.h"

namespace agoms {
namespace services {

// Outcome of applying a delay: the updated schedule plus any resource
// re-validation warnings that an Operations Manager should review.
struct DelayOutcome {
    FlightDelay delay;
    FlightSchedule updatedSchedule;
    std::vector<std::string> resourceWarnings; // non-empty if a previously
                                                // allocated resource is no
                                                // longer conflict-free for
                                                // the shifted time window
};

// Implements the delay-handling workflow from the specification:
//   1. Record delay reason & duration.
//   2. Update flight status to DELAYED.
//   3. Update affected schedule (shift estimated arrival/departure).
//   4. Re-check whether existing allocated resources are still conflict-free
//      for the new time window (resources are NOT auto-reallocated to a
//      different resource id in this academic implementation; instead the
//      Operations Manager is warned so they can manually reallocate via
//      AllocationService if needed).
class DelayService {
public:
    DelayService(repo::AirportDatabase& db, FlightService& flightService,
                 AllocationService& allocationService)
        : db_(db), flightService_(flightService), allocationService_(allocationService),
          conflictDetector_(db) {}

    DelayOutcome applyDelay(const std::string& flightId, DelayReason reason,
                             int delayMinutes, const std::string& notes);

    std::vector<FlightDelay> listDelaysForFlight(const std::string& flightId) const {
        return db_.listDelaysForFlight(flightId);
    }
    std::vector<FlightDelay> listAllDelays() const { return db_.listAllDelays(); }

private:
    repo::AirportDatabase& db_;
    FlightService& flightService_;
    AllocationService& allocationService_;
    ConflictDetector conflictDetector_;
    util::IdGenerator delayIdGen_{"DL", 1};
};

} // namespace services
} // namespace agoms

#endif // AGOMS_DELAY_SERVICE_H
