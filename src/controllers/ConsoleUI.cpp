#include "controllers/ConsoleUI.h"
#include "utils/Exceptions.h"

#include <iostream>
#include <sstream>
#include <limits>

namespace agoms {
namespace controllers {

using namespace services;

namespace {

// Thrown internally when stdin is closed/EOF while the UI is waiting for
// input (e.g. a piped script ran out of lines, or the terminal was closed).
// Caught only in ConsoleUI::run() to terminate the console loop cleanly
// instead of spinning forever re-reading empty lines.
struct EndOfInput {};

std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) {
        throw EndOfInput{};
    }
    return line;
}

int readInt(const std::string& prompt) {
    while (true) {
        std::string line = readLine(prompt);
        try {
            size_t idx;
            int value = std::stoi(line, &idx);
            if (idx == line.size()) return value;
        } catch (...) {}
        std::cout << "  Invalid number, please try again.\n";
    }
}

util::DateTime readDateTime(const std::string& prompt) {
    while (true) {
        std::string line = readLine(prompt + " (YYYY-MM-DD HH:MM): ");
        util::DateTime dt = util::DateTime::fromString(line);
        if (dt.raw() != 0) return dt;
        std::cout << "  Invalid date/time format, please try again.\n";
    }
}

std::vector<std::string> splitCommaList(const std::string& text) {
    std::vector<std::string> parts;
    std::stringstream ss(text);
    std::string item;
    while (std::getline(ss, item, ',')) {
        // trim whitespace
        size_t start = item.find_first_not_of(" \t");
        size_t end = item.find_last_not_of(" \t");
        if (start != std::string::npos) parts.push_back(item.substr(start, end - start + 1));
    }
    return parts;
}

TaskType readTaskType() {
    std::cout << "  Task type: 1) Baggage Handling 2) Aircraft Cleaning 3) Fueling 4) Catering 5) Boarding Support\n";
    int choice = readInt("  Select (1-5): ");
    switch (choice) {
        case 1: return TaskType::BAGGAGE_HANDLING;
        case 2: return TaskType::AIRCRAFT_CLEANING;
        case 3: return TaskType::FUELING;
        case 4: return TaskType::CATERING;
        default: return TaskType::BOARDING_SUPPORT;
    }
}

DelayReason readDelayReason() {
    std::cout << "  Delay reason: 1) Weather 2) ATC 3) Ground Operation 4) Resource Conflict 5) Technical Issue 6) Other\n";
    int choice = readInt("  Select (1-6): ");
    switch (choice) {
        case 1: return DelayReason::WEATHER;
        case 2: return DelayReason::ATC;
        case 3: return DelayReason::GROUND_OPERATION;
        case 4: return DelayReason::RESOURCE_CONFLICT;
        case 5: return DelayReason::TECHNICAL_ISSUE;
        default: return DelayReason::OTHER;
    }
}

FlightStatus readFlightStatus() {
    std::cout << "  Target status:\n"
              << "   1) APPROACHING  2) ARRIVED  3) GROUND_OPERATIONS  4) BOARDING\n"
              << "   5) READY_FOR_DEPARTURE  6) DEPARTED  7) COMPLETED  8) CANCELLED\n";
    int choice = readInt("  Select (1-8): ");
    switch (choice) {
        case 1: return FlightStatus::APPROACHING;
        case 2: return FlightStatus::ARRIVED;
        case 3: return FlightStatus::GROUND_OPERATIONS;
        case 4: return FlightStatus::BOARDING;
        case 5: return FlightStatus::READY_FOR_DEPARTURE;
        case 6: return FlightStatus::DEPARTED;
        case 7: return FlightStatus::COMPLETED;
        default: return FlightStatus::CANCELLED;
    }
}

} // namespace

ConsoleUI::ConsoleUI(repo::AirportDatabase& db)
    : db_(db),
      userService_(db),
      flightService_(db),
      resourceService_(db),
      allocationService_(db),
      atcService_(db),
      weatherService_(db),
      flightMonitor_(db, atcService_, weatherService_, flightService_),
      delayService_(db, flightService_, allocationService_),
      emergencyService_(db, flightService_),
      groundOpsService_(db),
      reportService_(db),
      currentUser_(nullptr) {}

bool ConsoleUI::requireLogin() {
    if (!currentUser_) {
        std::cout << "\n  You must log in first (Main Menu option 1).\n";
        return false;
    }
    return true;
}

bool ConsoleUI::requireRole(UserRole role, const std::string& actionDescription) {
    if (!requireLogin()) return false;
    if (currentUser_->getRole() != role) {
        std::cout << "\n  Access denied: '" << actionDescription << "' requires role "
                  << toString(role) << ", but you are logged in as "
                  << currentUser_->describeRole() << ".\n";
        return false;
    }
    return true;
}

void ConsoleUI::run() {
    std::cout << "==================================================\n";
    std::cout << " Airport Ground Operations Management System\n";
    std::cout << "==================================================\n";

    bool running = true;
    while (running) {
        showMainMenu();
        int choice;
        try {
            choice = readInt("Select an option: ");
        } catch (const EndOfInput&) {
            std::cout << "\n  End of input reached. Exiting.\n";
            break;
        }
        try {
            switch (choice) {
                case 1:  handleLogin(); break;
                case 2:  handleManageFlights(); break;
                case 3:  handleManageResources(); break;
                case 4:  handleAllocateResources(); break;
                case 5:  handleCheckConflicts(); break;
                case 6:  handleViewFlightStatus(); break;
                case 7:  handleManageGroundTasks(); break;
                case 8:  handleFlightDelay(); break;
                case 9:  handleEmergencyLanding(); break;
                case 10: handleViewWeather(); break;
                case 11: handleViewATC(); break;
                case 12: handleGenerateReports(); break;
                case 13:
                    std::cout << "\n  Goodbye!\n";
                    running = false;
                    break;
                default:
                    std::cout << "\n  Invalid option.\n";
            }
        } catch (const EndOfInput&) {
            std::cout << "\n  End of input reached. Exiting.\n";
            running = false;
        } catch (const AgomsException& ex) {
            std::cout << "\n  [ERROR] " << ex.what() << "\n";
        } catch (const std::exception& ex) {
            std::cout << "\n  [UNEXPECTED ERROR] " << ex.what() << "\n";
        }
    }
}

void ConsoleUI::showMainMenu() {
    std::cout << "\n-------------------- MAIN MENU --------------------\n";
    if (currentUser_) {
        std::cout << " Logged in as: " << currentUser_->getFullName()
                  << " (" << currentUser_->describeRole() << ")\n";
    } else {
        std::cout << " Not logged in.\n";
    }
    std::cout << " 1.  Login\n"
              << " 2.  Manage Flights\n"
              << " 3.  Manage Resources\n"
              << " 4.  Allocate Resources\n"
              << " 5.  Check Resource Conflicts\n"
              << " 6.  View Flight Status\n"
              << " 7.  Manage Ground Tasks\n"
              << " 8.  Handle Flight Delay\n"
              << " 9.  Handle Emergency Landing\n"
              << " 10. View Weather\n"
              << " 11. View ATC Status\n"
              << " 12. Generate Reports\n"
              << " 13. Exit\n"
              << "-----------------------------------------------------\n";
}

void ConsoleUI::handleLogin() {
    std::string username = readLine("Username: ");
    std::string password = readLine("Password: ");
    currentUser_ = userService_.login(username, password);
    std::cout << "\n  Login successful. Welcome, " << currentUser_->getFullName()
              << " (" << currentUser_->describeRole() << ").\n";
}
void ConsoleUI::handleManageFlights() {
    if (!requireLogin()) return;
    std::cout << "\n-- Manage Flights --\n"
              << " 1. Create new flight\n"
              << " 2. List all flights\n"
              << " 3. Transition flight status\n"
              << " 0. Back\n";
    int choice = readInt("Select: ");
    if (choice == 1) {
        if (!requireRole(UserRole::ADMINISTRATOR, "Create Flight")) return;
        std::string flightId = readLine("Flight ID (e.g. FL004): ");
        std::string flightNumber = readLine("Flight number (e.g. AI404): ");
        std::string origin = readLine("Origin: ");
        std::string destination = readLine("Destination: ");
        std::string aircraftType = readLine("Aircraft type: ");
        util::DateTime arrival = readDateTime("Scheduled arrival");
        util::DateTime departure = readDateTime("Scheduled departure");
        flightService_.createFlight(flightId, flightNumber, origin, destination, aircraftType, arrival, departure);
        std::cout << "\n  Flight " << flightId << " created.\n";
    } else if (choice == 2) {
        auto flights = flightService_.listFlights();
        std::cout << "\n  " << flights.size() << " flight(s):\n";
        for (const auto& f : flights) {
            std::cout << "   " << f.getFlightId() << " | " << f.getFlightNumber() << " | "
                      << f.getOrigin() << "->" << f.getDestination() << " | " << f.getAircraftType()
                      << " | Status: " << toString(f.getStatus()) << "\n";
        }
    } else if (choice == 3) {
        if (!requireRole(UserRole::OPERATIONS_MANAGER, "Transition Flight Status")) return;
        std::string flightId = readLine("Flight ID: ");
        FlightStatus newStatus = readFlightStatus();
        flightService_.transitionFlightStatus(flightId, newStatus);
        std::cout << "\n  Flight " << flightId << " is now " << toString(newStatus) << ".\n";
    }
}

void ConsoleUI::handleManageResources() {
    if (!requireLogin()) return;
    std::cout << "\n-- Manage Resources --\n"
              << " 1. Add gate\n 2. Add vehicle\n 3. Add ground staff resource\n"
              << " 4. List gates\n 5. List vehicles\n 6. List ground staff\n"
              << " 7. Set resource status (maintenance/available)\n 0. Back\n";
    int choice = readInt("Select: ");
    if (choice >= 1 && choice <= 3) {
        if (!requireRole(UserRole::ADMINISTRATOR, "Add Resource")) return;
    }
    switch (choice) {
        case 1: {
            std::string id = readLine("Gate ID (e.g. G04): ");
            std::string terminal = readLine("Terminal: ");
            resourceService_.addGate(id, terminal);
            std::cout << "\n  Gate " << id << " added.\n";
            break;
        }
        case 2: {
            std::string id = readLine("Vehicle ID (e.g. V03): ");
            std::string cls = readLine("Vehicle class: ");
            resourceService_.addVehicle(id, cls);
            std::cout << "\n  Vehicle " << id << " added.\n";
            break;
        }
        case 3: {
            std::string id = readLine("Ground staff resource ID (e.g. GS05): ");
            std::string spec = readLine("Specialization: ");
            resourceService_.addGroundStaffResource(id, spec);
            std::cout << "\n  Ground staff resource " << id << " added.\n";
            break;
        }
        case 4:
            std::cout << "\n  Gates:\n";
            for (const auto& g : resourceService_.listGates()) std::cout << "   " << g.describe() << "\n";
            break;
        case 5:
            std::cout << "\n  Vehicles:\n";
            for (const auto& v : resourceService_.listVehicles()) std::cout << "   " << v.describe() << "\n";
            break;
        case 6:
            std::cout << "\n  Ground Staff:\n";
            for (const auto& s : resourceService_.listGroundStaff()) std::cout << "   " << s.describe() << "\n";
            break;
        case 7: {
            if (!requireRole(UserRole::ADMINISTRATOR, "Set Resource Status")) return;
            std::string type = readLine("Resource type (gate/vehicle/staff): ");
            std::string id = readLine("Resource ID: ");
            std::cout << "  1) AVAILABLE 2) OCCUPIED 3) MAINTENANCE\n";
            int st = readInt("  Select: ");
            ResourceStatus status = st == 2 ? ResourceStatus::OCCUPIED :
                                     st == 3 ? ResourceStatus::MAINTENANCE : ResourceStatus::AVAILABLE;
            if (type == "gate") resourceService_.setGateStatus(id, status);
            else if (type == "vehicle") resourceService_.setVehicleStatus(id, status);
            else resourceService_.setGroundStaffStatus(id, status);
            std::cout << "\n  Resource " << id << " status updated to " << toString(status) << ".\n";
            break;
        }
        default: break;
    }
}
void ConsoleUI::handleAllocateResources() {
    if (!requireRole(UserRole::OPERATIONS_MANAGER, "Allocate Resources")) return;

    std::cout << "\n-- Allocate Resources --\n"
              << " 1. New allocation\n"
              << " 2. Release allocation (e.g. after ground operations complete)\n"
              << " 3. List allocations for a flight\n"
              << " 0. Back\n";
    int sub = readInt("Select: ");
    if (sub == 2) {
        std::string allocationId = readLine("Allocation ID to release: ");
        allocationService_.release(allocationId);
        std::cout << "\n  Allocation " << allocationId << " released. "
                     "Gate, vehicle and ground staff are now AVAILABLE again.\n";
        return;
    }
    if (sub == 3) {
        std::string flightId = readLine("Flight ID: ");
        auto allocs = allocationService_.listAllocationsForFlight(flightId);
        std::cout << "\n  " << allocs.size() << " allocation(s) for " << flightId << ":\n";
        for (const auto& a : allocs) {
            std::cout << "   " << a.getAllocationId() << " | Gate " << a.getGateId()
                      << " | Vehicle " << a.getVehicleId() << " | Status: " << toString(a.getStatus()) << "\n";
        }
        return;
    }
    if (sub != 1) return;

    AllocationRequest request;
    request.flightId = readLine("Flight ID: ");
    request.gateId = readLine("Gate ID: ");
    request.vehicleId = readLine("Vehicle ID: ");
    std::string staffCsv = readLine("Ground staff IDs (comma-separated, e.g. GS01,GS02): ");
    request.groundStaffIds = splitCommaList(staffCsv);
    request.windowStart = readDateTime("Window start");
    request.windowEnd = readDateTime("Window end");

    AllocationResult result = allocationService_.allocate(request);
    if (result.isSuccess()) {
        std::cout << "\n  Allocation Succeeded. Allocation ID: " << result.getAllocationId() << "\n";
        std::cout << "  Gate " << request.gateId << " -> OCCUPIED\n";
        std::cout << "  Vehicle " << request.vehicleId << " -> OCCUPIED\n";
        for (const auto& s : request.groundStaffIds) std::cout << "  Staff " << s << " -> OCCUPIED\n";
    } else {
        std::cout << "\n  Allocation Failed\n";
        std::cout << "  Conflict: " << result.getConflict().getDetails() << "\n";
    }
}

void ConsoleUI::handleCheckConflicts() {
    if (!requireLogin()) return;
    ConflictDetector detector(db_);

    AllocationRequest request;
    request.flightId = readLine("Flight ID: ");
    request.gateId = readLine("Gate ID: ");
    request.vehicleId = readLine("Vehicle ID: ");
    std::string staffCsv = readLine("Ground staff IDs (comma-separated): ");
    request.groundStaffIds = splitCommaList(staffCsv);
    request.windowStart = readDateTime("Window start");
    request.windowEnd = readDateTime("Window end");

    ConflictResult result = detector.checkConflict(request);
    if (result.hasConflict()) {
        std::cout << "\n  Conflict detected: [" << toString(result.getConflictType()) << "] "
                  << result.getDetails() << "\n";
    } else {
        std::cout << "\n  No conflict. These resources are available for the requested window.\n";
    }
}

void ConsoleUI::handleViewFlightStatus() {
    if (!requireLogin()) return;
    std::string flightId = readLine("Flight ID: ");
    MonitoringSnapshot snapshot = flightMonitor_.monitor(flightId);

    std::cout << "\n  Flight " << flightId << " status: " << toString(snapshot.flightStatus) << "\n";
    std::cout << "  ATC clearance: " << toString(snapshot.atcStatus.clearance)
              << " | Departure: " << snapshot.atcStatus.departureInfo
              << " | Arrival: " << snapshot.atcStatus.arrivalInfo << "\n";
    std::cout << "  Weather: " << toString(snapshot.weather.condition)
              << " | Temp: " << snapshot.weather.temperatureCelsius << "C"
              << " | Wind: " << snapshot.weather.windSpeedKmh << "km/h"
              << " | Visibility: " << snapshot.weather.visibilityKm << "km\n";
    std::cout << "  Weather alert: " << toString(snapshot.weatherAlert.level)
              << " - " << snapshot.weatherAlert.message << "\n";
    if (!snapshot.issues.empty()) {
        std::cout << "  Detected issues:\n";
        for (const auto& issue : snapshot.issues) std::cout << "   - " << issue << "\n";
    }

    auto schedule = flightService_.getSchedule(flightId);
    if (schedule.has_value()) {
        std::cout << "  Schedule: Arrival " << schedule->getEstimatedArrival().toString()
                  << " | Departure " << schedule->getEstimatedDeparture().toString() << "\n";
    }
}
void ConsoleUI::handleManageGroundTasks() {
    if (!requireLogin()) return;
    std::cout << "\n-- Manage Ground Tasks --\n"
              << " 1. Create task (Operations Manager)\n"
              << " 2. View my assigned tasks (Ground Staff)\n"
              << " 3. View tasks for a flight\n"
              << " 4. Update task status (Ground Staff)\n"
              << " 0. Back\n";
    int choice = readInt("Select: ");
    if (choice == 1) {
        if (!requireRole(UserRole::OPERATIONS_MANAGER, "Create Ground Task")) return;
        std::string flightId = readLine("Flight ID: ");
        TaskType type = readTaskType();
        std::string staffId = readLine("Assigned ground staff resource ID (blank for unassigned): ");
        util::DateTime start = readDateTime("Task start");
        util::DateTime end = readDateTime("Task end");
        GroundTask task = groundOpsService_.createTask(flightId, type, staffId, start, end);
        std::cout << "\n  Task " << task.getTaskId() << " created with status " << toString(task.getStatus()) << ".\n";
    } else if (choice == 2) {
        if (!requireRole(UserRole::GROUND_STAFF, "View Assigned Tasks")) return;
        const auto& gs = static_cast<const GroundStaff&>(*currentUser_);
        auto tasks = groundOpsService_.listTasksForStaff(gs.getLinkedResourceId());
        std::cout << "\n  " << tasks.size() << " task(s) assigned to you:\n";
        for (const auto& t : tasks) {
            std::cout << "   " << t.getTaskId() << " | Flight " << t.getFlightId() << " | "
                      << toString(t.getType()) << " | " << toString(t.getStatus()) << "\n";
        }
    } else if (choice == 3) {
        std::string flightId = readLine("Flight ID: ");
        auto tasks = groundOpsService_.listTasksForFlight(flightId);
        std::cout << "\n  " << tasks.size() << " task(s) for " << flightId << ":\n";
        for (const auto& t : tasks) {
            std::cout << "   " << t.getTaskId() << " | " << toString(t.getType()) << " | Staff: "
                      << (t.getAssignedStaffId().empty() ? "UNASSIGNED" : t.getAssignedStaffId())
                      << " | " << toString(t.getStatus()) << "\n";
        }
    } else if (choice == 4) {
        if (!requireRole(UserRole::GROUND_STAFF, "Update Task Status")) return;
        std::string taskId = readLine("Task ID: ");
        std::cout << "  1) PENDING 2) ASSIGNED 3) IN_PROGRESS 4) COMPLETED 5) DELAYED\n";
        int st = readInt("  Select: ");
        TaskStatus status = st == 1 ? TaskStatus::PENDING : st == 2 ? TaskStatus::ASSIGNED :
                             st == 3 ? TaskStatus::IN_PROGRESS : st == 4 ? TaskStatus::COMPLETED : TaskStatus::DELAYED;
        groundOpsService_.updateTaskStatus(taskId, status);
        std::cout << "\n  Task " << taskId << " updated to " << toString(status) << ".\n";
    }
}

void ConsoleUI::handleFlightDelay() {
    if (!requireRole(UserRole::OPERATIONS_MANAGER, "Handle Flight Delay")) return;
    std::string flightId = readLine("Flight ID: ");
    DelayReason reason = readDelayReason();
    int minutes = readInt("Delay duration (minutes): ");
    std::string notes = readLine("Notes (optional): ");

    DelayOutcome outcome = delayService_.applyDelay(flightId, reason, minutes, notes);
    std::cout << "\n  Delay recorded: " << outcome.delay.getDelayId() << "\n";
    std::cout << "  Flight " << flightId << " is now DELAYED.\n";
    std::cout << "  Updated schedule -> Arrival: " << outcome.updatedSchedule.getEstimatedArrival().toString()
              << " | Departure: " << outcome.updatedSchedule.getEstimatedDeparture().toString() << "\n";
    if (outcome.resourceWarnings.empty()) {
        std::cout << "  All previously allocated resources remain conflict-free for the new schedule.\n";
    } else {
        std::cout << "  WARNING - the following allocations may need reallocation:\n";
        for (const auto& w : outcome.resourceWarnings) std::cout << "   - " << w << "\n";
    }
}

void ConsoleUI::handleEmergencyLanding() {
    if (!requireRole(UserRole::OPERATIONS_MANAGER, "Handle Emergency Landing")) return;
    std::cout << "\n-- Emergency Landing --\n"
              << " 1. Declare emergency\n 2. Record landing\n 3. Begin ground handling\n"
              << " 4. Resolve emergency\n 5. List all emergencies\n 0. Back\n";
    int choice = readInt("Select: ");
    if (choice == 1) {
        std::string flightId = readLine("Flight ID: ");
        std::string reason = readLine("Emergency reason: ");
        EmergencyLanding emergency = emergencyService_.declareEmergency(flightId, reason);
        std::cout << "\n  Emergency declared: " << emergency.getEmergencyId()
                  << ". Flight " << flightId << " -> EMERGENCY_LANDING.\n";
    } else if (choice == 2) {
        std::string emergencyId = readLine("Emergency ID: ");
        EmergencyLanding emergency = emergencyService_.recordLanding(emergencyId);
        std::cout << "\n  Landing recorded at " << emergency.getLandingTime().toString() << ".\n";
    } else if (choice == 3) {
        std::string flightId = readLine("Flight ID: ");
        std::string emergencyId = readLine("Emergency ID: ");
        emergencyService_.beginGroundHandling(flightId, emergencyId);
        std::cout << "\n  Flight " << flightId << " -> GROUND_OPERATIONS. Emergency -> GROUND_HANDLING.\n";
    } else if (choice == 4) {
        std::string emergencyId = readLine("Emergency ID: ");
        emergencyService_.resolveEmergency(emergencyId);
        std::cout << "\n  Emergency " << emergencyId << " resolved.\n";
    } else if (choice == 5) {
        auto emergencies = emergencyService_.listAllEmergencies();
        std::cout << "\n  " << emergencies.size() << " emergency record(s):\n";
        for (const auto& e : emergencies) {
            std::cout << "   " << e.getEmergencyId() << " | Flight " << e.getFlightId()
                      << " | " << e.getReason() << " | " << toString(e.getStatus()) << "\n";
        }
    }
}
void ConsoleUI::handleViewWeather() {
    if (!requireLogin()) return;
    WeatherData data = weatherService_.getCurrentWeather();
    WeatherAlert alert = weatherService_.checkWeatherAlert(data);
    std::cout << "\n  Current Weather (simulated):\n";
    std::cout << "   Condition: " << toString(data.condition) << "\n";
    std::cout << "   Temperature: " << data.temperatureCelsius << " C\n";
    std::cout << "   Wind speed: " << data.windSpeedKmh << " km/h\n";
    std::cout << "   Visibility: " << data.visibilityKm << " km\n";
    std::cout << "   Precipitation: " << data.precipitationMm << " mm\n";
    std::cout << "   Alert level: " << toString(alert.level) << " - " << alert.message << "\n";
}

void ConsoleUI::handleViewATC() {
    if (!requireLogin()) return;
    std::string flightId = readLine("Flight ID: ");
    ATCStatus status = atcService_.getFlightStatus(flightId);
    std::cout << "\n  ATC Status for " << flightId << ":\n";
    std::cout << "   Clearance: " << toString(status.clearance) << "\n";
    std::cout << "   Departure info: " << status.departureInfo << "\n";
    std::cout << "   Arrival info: " << status.arrivalInfo << "\n";
    std::cout << "   Remarks: " << status.remarks << "\n";
}

void ConsoleUI::handleGenerateReports() {
    if (!requireLogin()) return;
    std::cout << "\n-- Generate Reports --\n"
              << " 1. Flight Status Report\n"
              << " 2. Resource Utilization Report\n"
              << " 3. Allocation Report\n"
              << " 4. Delay Report\n"
              << " 5. Emergency Report\n"
              << " 6. Ground Operation Report\n"
              << " 7. View previously saved reports\n"
              << " 0. Back\n";
    int choice = readInt("Select: ");
    Report* generated = nullptr;
    Report report("", ReportType::FLIGHT_STATUS, "");
    switch (choice) {
        case 1: report = reportService_.generateFlightStatusReport(); generated = &report; break;
        case 2: report = reportService_.generateResourceUtilizationReport(); generated = &report; break;
        case 3: report = reportService_.generateAllocationReport(); generated = &report; break;
        case 4: report = reportService_.generateDelayReport(); generated = &report; break;
        case 5: report = reportService_.generateEmergencyReport(); generated = &report; break;
        case 6: report = reportService_.generateGroundOperationReport(); generated = &report; break;
        case 7: {
            auto summaries = reportService_.listSavedReports();
            std::cout << "\n  Saved reports:\n";
            for (const auto& s : summaries) std::cout << "   " << s.first << " | " << s.second << "\n";
            if (!summaries.empty()) {
                std::string id = readLine("Enter report ID to view (blank to skip): ");
                if (!id.empty()) std::cout << "\n" << reportService_.getSavedReportContent(id) << "\n";
            }
            return;
        }
        default: return;
    }
    if (generated) std::cout << "\n" << generated->render() << "\n";
}

} // namespace controllers
} // namespace agoms
