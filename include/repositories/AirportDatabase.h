#ifndef AGOMS_AIRPORT_DATABASE_H
#define AGOMS_AIRPORT_DATABASE_H

#include <string>
#include <vector>
#include <memory>
#include <optional>

#include "models/User.h"
#include "models/AirportAdministrator.h"
#include "models/OperationsManager.h"
#include "models/GroundStaff.h"
#include "models/Flight.h"
#include "models/FlightSchedule.h"
#include "models/Gate.h"
#include "models/Vehicle.h"
#include "models/GroundStaffResource.h"
#include "models/Allocation.h"
#include "models/GroundTask.h"
#include "models/FlightDelay.h"
#include "models/EmergencyLanding.h"
#include "models/WeatherData.h"
#include "models/ATCStatus.h"
#include "models/Report.h"

// Forward declaration keeps the sqlite C API out of every translation unit
// that merely uses AirportDatabase (Pimpl-free, just a private struct
// pointer). See src/repositories/AirportDatabase.cpp for the real type.
struct sqlite3;

namespace agoms {
namespace repo {

// AirportDatabase is the single Repository-pattern gateway to persistence.
// It owns a real SQLite connection (see third_party/sqlite3/sqlite3_min.h
// for why this project hand-declares the SQLite C API instead of shipping
// the official header) and exposes typed, parameterized CRUD operations for
// every entity in the system. All SQL text lives in this .cpp file only —
// no other layer constructs SQL strings, which keeps the rest of the
// codebase injection-safe and easy to audit.
class AirportDatabase {
public:
    explicit AirportDatabase(const std::string& dbFilePath);
    ~AirportDatabase();

    AirportDatabase(const AirportDatabase&) = delete;
    AirportDatabase& operator=(const AirportDatabase&) = delete;

    // Movable (but not copyable) so factory-style helper functions can
    // return an AirportDatabase by value (used throughout the test suite).
    AirportDatabase(AirportDatabase&& other) noexcept;
    AirportDatabase& operator=(AirportDatabase&& other) noexcept;

    // Creates all tables (idempotent: "CREATE TABLE IF NOT EXISTS").
    void initializeSchema();

    // Populates sample data (flights/gates/vehicles/staff/users) the first
    // time the app runs against an empty database. Idempotent: does nothing
    // if the flights table is already non-empty.
    void seedSampleDataIfEmpty();

    // ---------------------------------------------------------------- Users
    void saveUser(const User& user);
    std::optional<std::unique_ptr<User>> findUserByUsername(const std::string& username);
    std::optional<std::unique_ptr<User>> findUserById(const std::string& userId);
    std::vector<std::unique_ptr<User>> listUsers();

    // -------------------------------------------------------------- Flights
    void saveFlight(const Flight& flight);
    void updateFlightStatus(const std::string& flightId, FlightStatus status);
    std::optional<Flight> findFlightById(const std::string& flightId);
    std::vector<Flight> listFlights();

    // ------------------------------------------------------ Flight schedule
    void saveFlightSchedule(const FlightSchedule& schedule);
    void updateScheduleEstimates(const std::string& flightId, util::DateTime estArrival,
                                  util::DateTime estDeparture);
    std::optional<FlightSchedule> findScheduleByFlightId(const std::string& flightId);

    // ----------------------------------------------------------- Resources
    void saveGate(const Gate& gate);
    void saveVehicle(const Vehicle& vehicle);
    void saveGroundStaffResource(const GroundStaffResource& resource);

    std::vector<Gate> listGates();
    std::vector<Vehicle> listVehicles();
    std::vector<GroundStaffResource> listGroundStaffResources();

    std::optional<Gate> findGate(const std::string& id);
    std::optional<Vehicle> findVehicle(const std::string& id);
    std::optional<GroundStaffResource> findGroundStaffResource(const std::string& id);

    void updateGateStatus(const std::string& id, ResourceStatus status);
    void updateVehicleStatus(const std::string& id, ResourceStatus status);
    void updateGroundStaffResourceStatus(const std::string& id, ResourceStatus status);

    // --------------------------------------------------------- Allocations
    void saveAllocation(const Allocation& allocation);
    void updateAllocationStatus(const std::string& allocationId, AllocationStatus status);
    std::optional<Allocation> findAllocationById(const std::string& allocationId);
    std::vector<Allocation> listActiveAllocationsForGate(const std::string& gateId);
    std::vector<Allocation> listActiveAllocationsForVehicle(const std::string& vehicleId);
    std::vector<Allocation> listActiveAllocationsForStaff(const std::string& staffId);
    std::vector<Allocation> listAllocationsForFlight(const std::string& flightId);
    std::vector<Allocation> listAllAllocations();

    // ------------------------------------------------------------- Tasks
    void saveGroundTask(const GroundTask& task);
    void updateTaskStatus(const std::string& taskId, TaskStatus status);
    std::vector<GroundTask> listTasksForFlight(const std::string& flightId);
    std::vector<GroundTask> listTasksForStaff(const std::string& staffId);
    std::vector<GroundTask> listAllTasks();

    // -------------------------------------------------------------- Delays
    void saveDelay(const FlightDelay& delay);
    std::vector<FlightDelay> listDelaysForFlight(const std::string& flightId);
    std::vector<FlightDelay> listAllDelays();

    // ---------------------------------------------------------- Emergency
    void saveEmergency(const EmergencyLanding& emergency);
    void updateEmergency(const EmergencyLanding& emergency);
    std::vector<EmergencyLanding> listAllEmergencies();

    // ----------------------------------------------------------- Weather
    void logWeather(const WeatherData& data);

    // --------------------------------------------------------------- ATC
    void logAtcStatus(const ATCStatus& status);

    // ----------------------------------------------------------- Reports
    void saveReport(const Report& report);
    std::vector<std::pair<std::string, std::string>> listReportSummaries(); // (id, title)
    std::optional<std::string> getReportContent(const std::string& reportId);

private:
    sqlite3* db_;
    std::string dbPath_;

    void execRaw(const std::string& sql);
};

} // namespace repo
} // namespace agoms

#endif // AGOMS_AIRPORT_DATABASE_H
