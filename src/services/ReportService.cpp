#include "services/ReportService.h"
#include "utils/Exceptions.h"
#include <sstream>

namespace agoms {
namespace services {

Report ReportService::finalize(Report report) {
    db_.saveReport(report);
    return report;
}

Report ReportService::generateFlightStatusReport() {
    Report report(reportIdGen_.next(), ReportType::FLIGHT_STATUS, "Flight Status Report");
    for (const auto& flight : db_.listFlights()) {
        std::ostringstream line;
        line << flight.getFlightId() << " (" << flight.getFlightNumber() << ") "
             << flight.getOrigin() << " -> " << flight.getDestination()
             << " | Status: " << toString(flight.getStatus());
        report.addLine(line.str());
    }
    return finalize(report);
}

Report ReportService::generateResourceUtilizationReport() {
    Report report(reportIdGen_.next(), ReportType::RESOURCE_UTILIZATION, "Resource Utilization Report");
    report.addLine("-- Gates --");
    for (const auto& g : db_.listGates()) report.addLine(g.describe());
    report.addLine("-- Vehicles --");
    for (const auto& v : db_.listVehicles()) report.addLine(v.describe());
    report.addLine("-- Ground Staff --");
    for (const auto& s : db_.listGroundStaffResources()) report.addLine(s.describe());
    return finalize(report);
}

Report ReportService::generateAllocationReport() {
    Report report(reportIdGen_.next(), ReportType::ALLOCATION, "Allocation Report");
    for (const auto& a : db_.listAllAllocations()) {
        std::ostringstream line;
        line << a.getAllocationId() << " | Flight " << a.getFlightId()
             << " | Gate " << a.getGateId() << " | Vehicle " << a.getVehicleId()
             << " | Staff [";
        const auto& staff = a.getGroundStaffIds();
        for (size_t i = 0; i < staff.size(); ++i) {
            line << staff[i];
            if (i + 1 < staff.size()) line << ", ";
        }
        line << "] | Window " << a.getWindowStart().toString() << " - " << a.getWindowEnd().toString()
             << " | Status: " << toString(a.getStatus());
        report.addLine(line.str());
    }
    return finalize(report);
}

Report ReportService::generateDelayReport() {
    Report report(reportIdGen_.next(), ReportType::DELAY, "Flight Delay Report");
    for (const auto& d : db_.listAllDelays()) {
        std::ostringstream line;
        line << d.getDelayId() << " | Flight " << d.getFlightId() << " | Reason: " << toString(d.getReason())
             << " | Duration: " << d.getDelayMinutes() << " min | Notes: " << d.getNotes();
        report.addLine(line.str());
    }
    return finalize(report);
}

Report ReportService::generateEmergencyReport() {
    Report report(reportIdGen_.next(), ReportType::EMERGENCY, "Emergency Landing Report");
    for (const auto& e : db_.listAllEmergencies()) {
        std::ostringstream line;
        line << e.getEmergencyId() << " | Flight " << e.getFlightId() << " | Reason: " << e.getReason()
             << " | Declared: " << e.getDeclaredAt().toString()
             << " | Landing: " << e.getLandingTime().toString()
             << " | Status: " << toString(e.getStatus());
        report.addLine(line.str());
    }
    return finalize(report);
}

Report ReportService::generateGroundOperationReport() {
    Report report(reportIdGen_.next(), ReportType::GROUND_OPERATION, "Ground Operation Report");
    for (const auto& t : db_.listAllTasks()) {
        std::ostringstream line;
        line << t.getTaskId() << " | Flight " << t.getFlightId() << " | Type: " << toString(t.getType())
             << " | Staff: " << (t.getAssignedStaffId().empty() ? "UNASSIGNED" : t.getAssignedStaffId())
             << " | Status: " << toString(t.getStatus())
             << " | Window " << t.getStartTime().toString() << " - " << t.getEndTime().toString();
        report.addLine(line.str());
    }
    return finalize(report);
}

std::string ReportService::getSavedReportContent(const std::string& reportId) const {
    auto content = db_.getReportContent(reportId);
    if (!content.has_value()) {
        throw NotFoundException("Report not found: " + reportId);
    }
    return content.value();
}

} // namespace services
} // namespace agoms
