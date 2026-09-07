#ifndef AGOMS_REPORT_SERVICE_H
#define AGOMS_REPORT_SERVICE_H

#include <string>
#include <vector>
#include "models/Report.h"
#include "repositories/AirportDatabase.h"

namespace agoms {
namespace services {

// Generates and persists Reports by pulling data from AirportDatabase.
// Each generate*() method builds a Report, saves it (so it can be recalled
// later via listSavedReports/getSavedReportContent), and returns it for
// immediate display.
class ReportService {
public:
    explicit ReportService(repo::AirportDatabase& db) : db_(db) {}

    Report generateFlightStatusReport();
    Report generateResourceUtilizationReport();
    Report generateAllocationReport();
    Report generateDelayReport();
    Report generateEmergencyReport();
    Report generateGroundOperationReport();

    std::vector<std::pair<std::string, std::string>> listSavedReports() const {
        return db_.listReportSummaries();
    }
    std::string getSavedReportContent(const std::string& reportId) const;

private:
    repo::AirportDatabase& db_;
    util::IdGenerator reportIdGen_{"RPT", 1};

    Report finalize(Report report);
};

} // namespace services
} // namespace agoms

#endif // AGOMS_REPORT_SERVICE_H
