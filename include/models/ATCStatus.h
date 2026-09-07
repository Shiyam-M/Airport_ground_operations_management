#ifndef AGOMS_ATC_STATUS_H
#define AGOMS_ATC_STATUS_H

#include <string>
#include "models/Enums.h"

namespace agoms {

// Simulated ATC status snapshot for a single flight.
struct ATCStatus {
    std::string flightId;
    ATCClearance clearance = ATCClearance::PENDING;
    std::string departureInfo;
    std::string arrivalInfo;
    std::string remarks;
};

} // namespace agoms

#endif // AGOMS_ATC_STATUS_H
