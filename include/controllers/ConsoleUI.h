#ifndef AGOMS_CONSOLE_UI_H
#define AGOMS_CONSOLE_UI_H

#include <memory>
#include <string>
#include "models/User.h"
#include "repositories/AirportDatabase.h"
#include "services/UserService.h"
#include "services/FlightService.h"
#include "services/ResourceService.h"
#include "services/AllocationService.h"
#include "services/ConflictDetector.h"
#include "services/ATCService.h"
#include "services/WeatherService.h"
#include "services/FlightMonitor.h"
#include "services/DelayService.h"
#include "services/EmergencyService.h"
#include "services/GroundOperationsService.h"
#include "services/ReportService.h"

namespace agoms {
namespace controllers {

// Presentation layer: a console-based menu system. ConsoleUI depends only
// on the service layer (never on repositories::AirportDatabase directly for
// business operations, and never constructs SQL) so the business logic
// remains fully testable and swappable behind a different UI later (e.g. a
// GUI or web front-end) without modification.
class ConsoleUI {
public:
    explicit ConsoleUI(repo::AirportDatabase& db);

    // Runs the interactive console loop until the user selects Exit.
    void run();

private:
    repo::AirportDatabase& db_;
    services::UserService userService_;
    services::FlightService flightService_;
    services::ResourceService resourceService_;
    services::AllocationService allocationService_;
    services::ATCService atcService_;
    services::WeatherService weatherService_;
    services::FlightMonitor flightMonitor_;
    services::DelayService delayService_;
    services::EmergencyService emergencyService_;
    services::GroundOperationsService groundOpsService_;
    services::ReportService reportService_;

    std::unique_ptr<User> currentUser_;

    void showMainMenu();
    void handleLogin();
    void handleManageFlights();
    void handleManageResources();
    void handleAllocateResources();
    void handleCheckConflicts();
    void handleViewFlightStatus();
    void handleManageGroundTasks();
    void handleFlightDelay();
    void handleEmergencyLanding();
    void handleViewWeather();
    void handleViewATC();
    void handleGenerateReports();

    bool requireLogin();
    bool requireRole(UserRole role, const std::string& actionDescription);
};

} // namespace controllers
} // namespace agoms

#endif // AGOMS_CONSOLE_UI_H
