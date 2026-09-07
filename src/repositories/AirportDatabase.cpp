#include "repositories/AirportDatabase.h"
#include "utils/Exceptions.h"
#include "../../third_party/sqlite3/sqlite3_min.h"

#include <sstream>
#include <cstring>

namespace agoms {
namespace repo {

namespace {

// Small RAII wrapper around sqlite3_stmt* that removes the boilerplate of
// prepare/bind/step/finalize from every repository method below.
class Stmt {
public:
    Stmt(sqlite3* db, const std::string& sql) : db_(db), stmt_(nullptr) {
        int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt_, nullptr);
        if (rc != SQLITE_OK) {
            throw DatabaseException(std::string("Failed to prepare statement: ") + sqlite3_errmsg(db_));
        }
    }
    ~Stmt() {
        if (stmt_) sqlite3_finalize(stmt_);
    }
    Stmt(const Stmt&) = delete;
    Stmt& operator=(const Stmt&) = delete;

    void bindText(int index, const std::string& value) {
        sqlite3_bind_text(stmt_, index, value.c_str(), -1, SQLITE_TRANSIENT);
    }
    void bindInt(int index, int value) { sqlite3_bind_int(stmt_, index, value); }

    // Executes a statement expected to produce no rows (INSERT/UPDATE/DELETE).
    void run() {
        int rc = sqlite3_step(stmt_);
        if (rc != SQLITE_DONE) {
            throw DatabaseException(std::string("Statement execution failed: ") + sqlite3_errmsg(db_));
        }
    }

    // Advances to the next row; returns false when no more rows.
    bool next() {
        int rc = sqlite3_step(stmt_);
        if (rc == SQLITE_ROW) return true;
        if (rc == SQLITE_DONE) return false;
        throw DatabaseException(std::string("Statement step failed: ") + sqlite3_errmsg(db_));
    }

    std::string colText(int index) const {
        const unsigned char* text = sqlite3_column_text(stmt_, index);
        return text ? std::string(reinterpret_cast<const char*>(text)) : std::string();
    }
    int colInt(int index) const { return sqlite3_column_int(stmt_, index); }

    sqlite3_stmt* raw() { return stmt_; }

private:
    sqlite3* db_;
    sqlite3_stmt* stmt_;
};

std::string statusToStr(ResourceStatus s) { return toString(s); }

ResourceStatus parseResourceStatus(const std::string& s) {
    if (s == "AVAILABLE") return ResourceStatus::AVAILABLE;
    if (s == "OCCUPIED") return ResourceStatus::OCCUPIED;
    if (s == "MAINTENANCE") return ResourceStatus::MAINTENANCE;
    return ResourceStatus::AVAILABLE;
}

FlightStatus parseFlightStatus(const std::string& s) {
    if (s == "SCHEDULED") return FlightStatus::SCHEDULED;
    if (s == "APPROACHING") return FlightStatus::APPROACHING;
    if (s == "ARRIVED") return FlightStatus::ARRIVED;
    if (s == "EMERGENCY_LANDING") return FlightStatus::EMERGENCY_LANDING;
    if (s == "GROUND_OPERATIONS") return FlightStatus::GROUND_OPERATIONS;
    if (s == "BOARDING") return FlightStatus::BOARDING;
    if (s == "READY_FOR_DEPARTURE") return FlightStatus::READY_FOR_DEPARTURE;
    if (s == "DEPARTED") return FlightStatus::DEPARTED;
    if (s == "COMPLETED") return FlightStatus::COMPLETED;
    if (s == "DELAYED") return FlightStatus::DELAYED;
    if (s == "CANCELLED") return FlightStatus::CANCELLED;
    return FlightStatus::SCHEDULED;
}

AllocationStatus parseAllocationStatus(const std::string& s) {
    if (s == "SUCCESS") return AllocationStatus::SUCCESS;
    if (s == "RELEASED") return AllocationStatus::RELEASED;
    return AllocationStatus::FAILED;
}

TaskStatus parseTaskStatus(const std::string& s) {
    if (s == "PENDING") return TaskStatus::PENDING;
    if (s == "ASSIGNED") return TaskStatus::ASSIGNED;
    if (s == "IN_PROGRESS") return TaskStatus::IN_PROGRESS;
    if (s == "COMPLETED") return TaskStatus::COMPLETED;
    if (s == "DELAYED") return TaskStatus::DELAYED;
    return TaskStatus::PENDING;
}

TaskType parseTaskType(const std::string& s) {
    if (s == "BAGGAGE_HANDLING") return TaskType::BAGGAGE_HANDLING;
    if (s == "AIRCRAFT_CLEANING") return TaskType::AIRCRAFT_CLEANING;
    if (s == "FUELING") return TaskType::FUELING;
    if (s == "CATERING") return TaskType::CATERING;
    if (s == "BOARDING_SUPPORT") return TaskType::BOARDING_SUPPORT;
    return TaskType::BAGGAGE_HANDLING;
}

DelayReason parseDelayReason(const std::string& s) {
    if (s == "WEATHER") return DelayReason::WEATHER;
    if (s == "ATC") return DelayReason::ATC;
    if (s == "GROUND_OPERATION") return DelayReason::GROUND_OPERATION;
    if (s == "RESOURCE_CONFLICT") return DelayReason::RESOURCE_CONFLICT;
    if (s == "TECHNICAL_ISSUE") return DelayReason::TECHNICAL_ISSUE;
    return DelayReason::OTHER;
}

EmergencyStatus parseEmergencyStatus(const std::string& s) {
    if (s == "DECLARED") return EmergencyStatus::DECLARED;
    if (s == "LANDED") return EmergencyStatus::LANDED;
    if (s == "GROUND_HANDLING") return EmergencyStatus::GROUND_HANDLING;
    return EmergencyStatus::RESOLVED;
}

UserRole parseUserRole(const std::string& s) {
    if (s == "ADMINISTRATOR") return UserRole::ADMINISTRATOR;
    if (s == "OPERATIONS_MANAGER") return UserRole::OPERATIONS_MANAGER;
    return UserRole::GROUND_STAFF;
}

} // namespace

AirportDatabase::AirportDatabase(const std::string& dbFilePath) : db_(nullptr), dbPath_(dbFilePath) {
    int rc = sqlite3_open(dbFilePath.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::string msg = db_ ? sqlite3_errmsg(db_) : "unknown error";
        throw DatabaseException("Could not open database '" + dbFilePath + "': " + msg);
    }
    execRaw("PRAGMA foreign_keys = ON;");
}

AirportDatabase::~AirportDatabase() {
    if (db_) sqlite3_close(db_);
}

AirportDatabase::AirportDatabase(AirportDatabase&& other) noexcept
    : db_(other.db_), dbPath_(std::move(other.dbPath_)) {
    other.db_ = nullptr;
}

AirportDatabase& AirportDatabase::operator=(AirportDatabase&& other) noexcept {
    if (this != &other) {
        if (db_) sqlite3_close(db_);
        db_ = other.db_;
        dbPath_ = std::move(other.dbPath_);
        other.db_ = nullptr;
    }
    return *this;
}

void AirportDatabase::execRaw(const std::string& sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string msg = errMsg ? errMsg : "unknown error";
        if (errMsg) sqlite3_free(errMsg);
        throw DatabaseException("SQL execution failed: " + msg + "\nSQL: " + sql);
    }
}

void AirportDatabase::initializeSchema() {
    execRaw(R"SQL(
CREATE TABLE IF NOT EXISTS users (
    user_id TEXT PRIMARY KEY, username TEXT NOT NULL UNIQUE, password_hash TEXT NOT NULL,
    full_name TEXT NOT NULL, role TEXT NOT NULL, linked_resource_id TEXT, active INTEGER NOT NULL DEFAULT 1
);
CREATE TABLE IF NOT EXISTS flights (
    flight_id TEXT PRIMARY KEY, flight_number TEXT NOT NULL, origin TEXT NOT NULL,
    destination TEXT NOT NULL, aircraft_type TEXT NOT NULL, status TEXT NOT NULL DEFAULT 'SCHEDULED'
);
CREATE TABLE IF NOT EXISTS flight_schedules (
    schedule_id TEXT PRIMARY KEY, flight_id TEXT NOT NULL REFERENCES flights(flight_id),
    scheduled_arrival TEXT NOT NULL, scheduled_departure TEXT NOT NULL,
    estimated_arrival TEXT NOT NULL, estimated_departure TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS gates (
    resource_id TEXT PRIMARY KEY, terminal TEXT NOT NULL, status TEXT NOT NULL DEFAULT 'AVAILABLE'
);
CREATE TABLE IF NOT EXISTS vehicles (
    resource_id TEXT PRIMARY KEY, vehicle_class TEXT NOT NULL, status TEXT NOT NULL DEFAULT 'AVAILABLE'
);
CREATE TABLE IF NOT EXISTS ground_staff_resources (
    resource_id TEXT PRIMARY KEY, specialization TEXT NOT NULL, status TEXT NOT NULL DEFAULT 'AVAILABLE'
);
CREATE TABLE IF NOT EXISTS allocations (
    allocation_id TEXT PRIMARY KEY, flight_id TEXT NOT NULL REFERENCES flights(flight_id),
    gate_id TEXT NOT NULL REFERENCES gates(resource_id), vehicle_id TEXT NOT NULL REFERENCES vehicles(resource_id),
    window_start TEXT NOT NULL, window_end TEXT NOT NULL, status TEXT NOT NULL DEFAULT 'SUCCESS'
);
CREATE TABLE IF NOT EXISTS allocation_staff (
    allocation_id TEXT NOT NULL REFERENCES allocations(allocation_id),
    staff_id TEXT NOT NULL REFERENCES ground_staff_resources(resource_id),
    PRIMARY KEY (allocation_id, staff_id)
);
CREATE TABLE IF NOT EXISTS ground_tasks (
    task_id TEXT PRIMARY KEY, flight_id TEXT NOT NULL REFERENCES flights(flight_id), task_type TEXT NOT NULL,
    assigned_staff TEXT, status TEXT NOT NULL DEFAULT 'PENDING', start_time TEXT NOT NULL, end_time TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS flight_delays (
    delay_id TEXT PRIMARY KEY, flight_id TEXT NOT NULL REFERENCES flights(flight_id), reason TEXT NOT NULL,
    delay_minutes INTEGER NOT NULL, notes TEXT, recorded_at TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS emergency_landings (
    emergency_id TEXT PRIMARY KEY, flight_id TEXT NOT NULL REFERENCES flights(flight_id), reason TEXT NOT NULL,
    declared_at TEXT NOT NULL, landing_time TEXT, status TEXT NOT NULL DEFAULT 'DECLARED'
);
CREATE TABLE IF NOT EXISTS weather_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT, temperature_c REAL NOT NULL, wind_speed_kmh REAL NOT NULL,
    visibility_km REAL NOT NULL, precipitation_mm REAL NOT NULL, condition TEXT NOT NULL, observed_at TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS atc_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT, flight_id TEXT NOT NULL, clearance TEXT NOT NULL,
    departure_info TEXT, arrival_info TEXT, remarks TEXT, logged_at TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS reports (
    report_id TEXT PRIMARY KEY, report_type TEXT NOT NULL, title TEXT NOT NULL,
    generated_at TEXT NOT NULL, content TEXT NOT NULL
);
)SQL");
}

void AirportDatabase::seedSampleDataIfEmpty() {
    Stmt check(db_, "SELECT COUNT(*) FROM flights;");
    check.next();
    int count = check.colInt(0);
    if (count > 0) return; // already seeded

    // Users (Administrator / Operations Manager / two Ground Staff accounts)
    AirportAdministrator admin("U0001", "admin", "", "Alice Admin");
    admin.setPassword("admin123");
    saveUser(admin);

    OperationsManager opsMgr("U0002", "opsmgr", "", "Oscar Operations");
    opsMgr.setPassword("ops123");
    saveUser(opsMgr);

    GroundStaff staff1("U0003", "staff1", "", "Gina Ground", "GS01");
    staff1.setPassword("staff123");
    saveUser(staff1);

    GroundStaff staff2("U0004", "staff2", "", "Sam Ground", "GS02");
    staff2.setPassword("staff123");
    saveUser(staff2);

    // Gates
    saveGate(Gate("G01", "A"));
    saveGate(Gate("G02", "A"));
    saveGate(Gate("G03", "B"));

    // Vehicles
    saveVehicle(Vehicle("V01", "Baggage Tractor"));
    saveVehicle(Vehicle("V02", "Fuel Truck"));

    // Ground staff resources
    saveGroundStaffResource(GroundStaffResource("GS01", "Baggage Handling"));
    saveGroundStaffResource(GroundStaffResource("GS02", "Ramp Agent"));
    saveGroundStaffResource(GroundStaffResource("GS03", "Cleaning Crew"));
    saveGroundStaffResource(GroundStaffResource("GS04", "Catering Crew"));

    // Flights
    saveFlight(Flight("FL001", "AI101", "DEL", "BOM", "A320"));
    saveFlight(Flight("FL002", "AI202", "BOM", "BLR", "B737"));
    saveFlight(Flight("FL003", "AI303", "BLR", "MAA", "A321"));

    // Flight schedules
    saveFlightSchedule(FlightSchedule("SC0001", "FL001",
        util::DateTime::fromString("2026-08-26 09:00"), util::DateTime::fromString("2026-08-26 10:30")));
    saveFlightSchedule(FlightSchedule("SC0002", "FL002",
        util::DateTime::fromString("2026-08-26 11:00"), util::DateTime::fromString("2026-08-26 12:30")));
    saveFlightSchedule(FlightSchedule("SC0003", "FL003",
        util::DateTime::fromString("2026-08-26 13:00"), util::DateTime::fromString("2026-08-26 14:30")));
}
// ---------------------------------------------------------------- Users ---

void AirportDatabase::saveUser(const User& user) {
    Stmt stmt(db_, R"(
        INSERT INTO users (user_id, username, password_hash, full_name, role, linked_resource_id, active)
        VALUES (?, ?, ?, ?, ?, ?, ?)
        ON CONFLICT(user_id) DO UPDATE SET
            username=excluded.username, password_hash=excluded.password_hash,
            full_name=excluded.full_name, role=excluded.role,
            linked_resource_id=excluded.linked_resource_id, active=excluded.active;
    )");
    stmt.bindText(1, user.getUserId());
    stmt.bindText(2, user.getUsername());
    stmt.bindText(3, user.getPasswordHash());
    stmt.bindText(4, user.getFullName());
    stmt.bindText(5, toString(user.getRole()));
    std::string linkedId;
    if (user.getRole() == UserRole::GROUND_STAFF) {
        linkedId = static_cast<const GroundStaff&>(user).getLinkedResourceId();
    }
    stmt.bindText(6, linkedId);
    stmt.bindInt(7, user.isActive() ? 1 : 0);
    stmt.run();
}

namespace {
std::unique_ptr<User> rowToUser(Stmt& stmt) {
    std::string userId = stmt.colText(0);
    std::string username = stmt.colText(1);
    std::string passwordHash = stmt.colText(2);
    std::string fullName = stmt.colText(3);
    UserRole role = parseUserRole(stmt.colText(4));
    std::string linkedResourceId = stmt.colText(5);
    bool active = stmt.colInt(6) != 0;

    std::unique_ptr<User> user;
    switch (role) {
        case UserRole::ADMINISTRATOR:
            user = std::make_unique<AirportAdministrator>(userId, username, passwordHash, fullName);
            break;
        case UserRole::OPERATIONS_MANAGER:
            user = std::make_unique<OperationsManager>(userId, username, passwordHash, fullName);
            break;
        case UserRole::GROUND_STAFF:
            user = std::make_unique<GroundStaff>(userId, username, passwordHash, fullName, linkedResourceId);
            break;
    }
    user->setActive(active);
    return user;
}
} // namespace

std::optional<std::unique_ptr<User>> AirportDatabase::findUserByUsername(const std::string& username) {
    Stmt stmt(db_, "SELECT user_id, username, password_hash, full_name, role, linked_resource_id, active "
                   "FROM users WHERE username = ?;");
    stmt.bindText(1, username);
    if (!stmt.next()) return std::nullopt;
    return rowToUser(stmt);
}

std::optional<std::unique_ptr<User>> AirportDatabase::findUserById(const std::string& userId) {
    Stmt stmt(db_, "SELECT user_id, username, password_hash, full_name, role, linked_resource_id, active "
                   "FROM users WHERE user_id = ?;");
    stmt.bindText(1, userId);
    if (!stmt.next()) return std::nullopt;
    return rowToUser(stmt);
}

std::vector<std::unique_ptr<User>> AirportDatabase::listUsers() {
    std::vector<std::unique_ptr<User>> result;
    Stmt stmt(db_, "SELECT user_id, username, password_hash, full_name, role, linked_resource_id, active FROM users;");
    while (stmt.next()) {
        result.push_back(rowToUser(stmt));
    }
    return result;
}
// -------------------------------------------------------------- Flights ---

void AirportDatabase::saveFlight(const Flight& flight) {
    Stmt stmt(db_, R"(
        INSERT INTO flights (flight_id, flight_number, origin, destination, aircraft_type, status)
        VALUES (?, ?, ?, ?, ?, ?)
        ON CONFLICT(flight_id) DO UPDATE SET
            flight_number=excluded.flight_number, origin=excluded.origin,
            destination=excluded.destination, aircraft_type=excluded.aircraft_type, status=excluded.status;
    )");
    stmt.bindText(1, flight.getFlightId());
    stmt.bindText(2, flight.getFlightNumber());
    stmt.bindText(3, flight.getOrigin());
    stmt.bindText(4, flight.getDestination());
    stmt.bindText(5, flight.getAircraftType());
    stmt.bindText(6, toString(flight.getStatus()));
    stmt.run();
}

void AirportDatabase::updateFlightStatus(const std::string& flightId, FlightStatus status) {
    Stmt stmt(db_, "UPDATE flights SET status = ? WHERE flight_id = ?;");
    stmt.bindText(1, toString(status));
    stmt.bindText(2, flightId);
    stmt.run();
}

namespace {
Flight rowToFlight(Stmt& stmt) {
    Flight f(stmt.colText(0), stmt.colText(1), stmt.colText(2), stmt.colText(3), stmt.colText(4));
    f.setStatusUnchecked(parseFlightStatus(stmt.colText(5)));
    return f;
}
}

std::optional<Flight> AirportDatabase::findFlightById(const std::string& flightId) {
    Stmt stmt(db_, "SELECT flight_id, flight_number, origin, destination, aircraft_type, status "
                   "FROM flights WHERE flight_id = ?;");
    stmt.bindText(1, flightId);
    if (!stmt.next()) return std::nullopt;
    return rowToFlight(stmt);
}

std::vector<Flight> AirportDatabase::listFlights() {
    std::vector<Flight> result;
    Stmt stmt(db_, "SELECT flight_id, flight_number, origin, destination, aircraft_type, status FROM flights;");
    while (stmt.next()) result.push_back(rowToFlight(stmt));
    return result;
}

// ------------------------------------------------------ Flight schedule ---

void AirportDatabase::saveFlightSchedule(const FlightSchedule& schedule) {
    Stmt stmt(db_, R"(
        INSERT INTO flight_schedules (schedule_id, flight_id, scheduled_arrival, scheduled_departure,
                                       estimated_arrival, estimated_departure)
        VALUES (?, ?, ?, ?, ?, ?)
        ON CONFLICT(schedule_id) DO UPDATE SET
            estimated_arrival=excluded.estimated_arrival, estimated_departure=excluded.estimated_departure;
    )");
    stmt.bindText(1, schedule.getScheduleId());
    stmt.bindText(2, schedule.getFlightId());
    stmt.bindText(3, schedule.getScheduledArrival().toString());
    stmt.bindText(4, schedule.getScheduledDeparture().toString());
    stmt.bindText(5, schedule.getEstimatedArrival().toString());
    stmt.bindText(6, schedule.getEstimatedDeparture().toString());
    stmt.run();
}

void AirportDatabase::updateScheduleEstimates(const std::string& flightId, util::DateTime estArrival,
                                               util::DateTime estDeparture) {
    Stmt stmt(db_, "UPDATE flight_schedules SET estimated_arrival = ?, estimated_departure = ? WHERE flight_id = ?;");
    stmt.bindText(1, estArrival.toString());
    stmt.bindText(2, estDeparture.toString());
    stmt.bindText(3, flightId);
    stmt.run();
}

std::optional<FlightSchedule> AirportDatabase::findScheduleByFlightId(const std::string& flightId) {
    Stmt stmt(db_, "SELECT schedule_id, flight_id, scheduled_arrival, scheduled_departure, "
                   "estimated_arrival, estimated_departure FROM flight_schedules WHERE flight_id = ?;");
    stmt.bindText(1, flightId);
    if (!stmt.next()) return std::nullopt;
    FlightSchedule sched(stmt.colText(0), stmt.colText(1),
                          util::DateTime::fromString(stmt.colText(2)),
                          util::DateTime::fromString(stmt.colText(3)));
    sched.setEstimatedArrival(util::DateTime::fromString(stmt.colText(4)));
    sched.setEstimatedDeparture(util::DateTime::fromString(stmt.colText(5)));
    return sched;
}
// ----------------------------------------------------------- Resources ---

void AirportDatabase::saveGate(const Gate& gate) {
    Stmt stmt(db_, R"(
        INSERT INTO gates (resource_id, terminal, status) VALUES (?, ?, ?)
        ON CONFLICT(resource_id) DO UPDATE SET terminal=excluded.terminal, status=excluded.status;
    )");
    stmt.bindText(1, gate.getResourceId());
    stmt.bindText(2, gate.getTerminal());
    stmt.bindText(3, toString(gate.getStatus()));
    stmt.run();
}

void AirportDatabase::saveVehicle(const Vehicle& vehicle) {
    Stmt stmt(db_, R"(
        INSERT INTO vehicles (resource_id, vehicle_class, status) VALUES (?, ?, ?)
        ON CONFLICT(resource_id) DO UPDATE SET vehicle_class=excluded.vehicle_class, status=excluded.status;
    )");
    stmt.bindText(1, vehicle.getResourceId());
    stmt.bindText(2, vehicle.getVehicleClass());
    stmt.bindText(3, toString(vehicle.getStatus()));
    stmt.run();
}

void AirportDatabase::saveGroundStaffResource(const GroundStaffResource& resource) {
    Stmt stmt(db_, R"(
        INSERT INTO ground_staff_resources (resource_id, specialization, status) VALUES (?, ?, ?)
        ON CONFLICT(resource_id) DO UPDATE SET specialization=excluded.specialization, status=excluded.status;
    )");
    stmt.bindText(1, resource.getResourceId());
    stmt.bindText(2, resource.getSpecialization());
    stmt.bindText(3, toString(resource.getStatus()));
    stmt.run();
}

std::vector<Gate> AirportDatabase::listGates() {
    std::vector<Gate> result;
    Stmt stmt(db_, "SELECT resource_id, terminal, status FROM gates;");
    while (stmt.next()) {
        Gate g(stmt.colText(0), stmt.colText(1));
        g.setStatus(parseResourceStatus(stmt.colText(2)));
        result.push_back(g);
    }
    return result;
}

std::vector<Vehicle> AirportDatabase::listVehicles() {
    std::vector<Vehicle> result;
    Stmt stmt(db_, "SELECT resource_id, vehicle_class, status FROM vehicles;");
    while (stmt.next()) {
        Vehicle v(stmt.colText(0), stmt.colText(1));
        v.setStatus(parseResourceStatus(stmt.colText(2)));
        result.push_back(v);
    }
    return result;
}

std::vector<GroundStaffResource> AirportDatabase::listGroundStaffResources() {
    std::vector<GroundStaffResource> result;
    Stmt stmt(db_, "SELECT resource_id, specialization, status FROM ground_staff_resources;");
    while (stmt.next()) {
        GroundStaffResource r(stmt.colText(0), stmt.colText(1));
        r.setStatus(parseResourceStatus(stmt.colText(2)));
        result.push_back(r);
    }
    return result;
}

std::optional<Gate> AirportDatabase::findGate(const std::string& id) {
    Stmt stmt(db_, "SELECT resource_id, terminal, status FROM gates WHERE resource_id = ?;");
    stmt.bindText(1, id);
    if (!stmt.next()) return std::nullopt;
    Gate g(stmt.colText(0), stmt.colText(1));
    g.setStatus(parseResourceStatus(stmt.colText(2)));
    return g;
}

std::optional<Vehicle> AirportDatabase::findVehicle(const std::string& id) {
    Stmt stmt(db_, "SELECT resource_id, vehicle_class, status FROM vehicles WHERE resource_id = ?;");
    stmt.bindText(1, id);
    if (!stmt.next()) return std::nullopt;
    Vehicle v(stmt.colText(0), stmt.colText(1));
    v.setStatus(parseResourceStatus(stmt.colText(2)));
    return v;
}

std::optional<GroundStaffResource> AirportDatabase::findGroundStaffResource(const std::string& id) {
    Stmt stmt(db_, "SELECT resource_id, specialization, status FROM ground_staff_resources WHERE resource_id = ?;");
    stmt.bindText(1, id);
    if (!stmt.next()) return std::nullopt;
    GroundStaffResource r(stmt.colText(0), stmt.colText(1));
    r.setStatus(parseResourceStatus(stmt.colText(2)));
    return r;
}

void AirportDatabase::updateGateStatus(const std::string& id, ResourceStatus status) {
    Stmt stmt(db_, "UPDATE gates SET status = ? WHERE resource_id = ?;");
    stmt.bindText(1, statusToStr(status));
    stmt.bindText(2, id);
    stmt.run();
}

void AirportDatabase::updateVehicleStatus(const std::string& id, ResourceStatus status) {
    Stmt stmt(db_, "UPDATE vehicles SET status = ? WHERE resource_id = ?;");
    stmt.bindText(1, statusToStr(status));
    stmt.bindText(2, id);
    stmt.run();
}

void AirportDatabase::updateGroundStaffResourceStatus(const std::string& id, ResourceStatus status) {
    Stmt stmt(db_, "UPDATE ground_staff_resources SET status = ? WHERE resource_id = ?;");
    stmt.bindText(1, statusToStr(status));
    stmt.bindText(2, id);
    stmt.run();
}
// --------------------------------------------------------- Allocations ---

void AirportDatabase::saveAllocation(const Allocation& allocation) {
    Stmt stmt(db_, R"(
        INSERT INTO allocations (allocation_id, flight_id, gate_id, vehicle_id, window_start, window_end, status)
        VALUES (?, ?, ?, ?, ?, ?, ?);
    )");
    stmt.bindText(1, allocation.getAllocationId());
    stmt.bindText(2, allocation.getFlightId());
    stmt.bindText(3, allocation.getGateId());
    stmt.bindText(4, allocation.getVehicleId());
    stmt.bindText(5, allocation.getWindowStart().toString());
    stmt.bindText(6, allocation.getWindowEnd().toString());
    stmt.bindText(7, toString(allocation.getStatus()));
    stmt.run();

    for (const auto& staffId : allocation.getGroundStaffIds()) {
        Stmt link(db_, "INSERT INTO allocation_staff (allocation_id, staff_id) VALUES (?, ?);");
        link.bindText(1, allocation.getAllocationId());
        link.bindText(2, staffId);
        link.run();
    }
}

void AirportDatabase::updateAllocationStatus(const std::string& allocationId, AllocationStatus status) {
    Stmt stmt(db_, "UPDATE allocations SET status = ? WHERE allocation_id = ?;");
    stmt.bindText(1, toString(status));
    stmt.bindText(2, allocationId);
    stmt.run();
}

namespace {
std::vector<std::string> loadStaffForAllocation(sqlite3* db, const std::string& allocationId) {
    std::vector<std::string> staffIds;
    Stmt stmt(db, "SELECT staff_id FROM allocation_staff WHERE allocation_id = ?;");
    stmt.bindText(1, allocationId);
    while (stmt.next()) staffIds.push_back(stmt.colText(0));
    return staffIds;
}

Allocation rowToAllocation(sqlite3* db, Stmt& stmt) {
    std::string allocationId = stmt.colText(0);
    Allocation a(allocationId, stmt.colText(1), stmt.colText(2), stmt.colText(3),
                 loadStaffForAllocation(db, allocationId),
                 util::DateTime::fromString(stmt.colText(4)),
                 util::DateTime::fromString(stmt.colText(5)));
    if (parseAllocationStatus(stmt.colText(6)) == AllocationStatus::RELEASED) a.release();
    return a;
}
}

std::optional<Allocation> AirportDatabase::findAllocationById(const std::string& allocationId) {
    Stmt stmt(db_, "SELECT allocation_id, flight_id, gate_id, vehicle_id, window_start, window_end, status "
                   "FROM allocations WHERE allocation_id = ?;");
    stmt.bindText(1, allocationId);
    if (!stmt.next()) return std::nullopt;
    return rowToAllocation(db_, stmt);
}

std::vector<Allocation> AirportDatabase::listActiveAllocationsForGate(const std::string& gateId) {
    std::vector<Allocation> result;
    Stmt stmt(db_, "SELECT allocation_id, flight_id, gate_id, vehicle_id, window_start, window_end, status "
                   "FROM allocations WHERE gate_id = ? AND status = 'SUCCESS';");
    stmt.bindText(1, gateId);
    while (stmt.next()) result.push_back(rowToAllocation(db_, stmt));
    return result;
}

std::vector<Allocation> AirportDatabase::listActiveAllocationsForVehicle(const std::string& vehicleId) {
    std::vector<Allocation> result;
    Stmt stmt(db_, "SELECT allocation_id, flight_id, gate_id, vehicle_id, window_start, window_end, status "
                   "FROM allocations WHERE vehicle_id = ? AND status = 'SUCCESS';");
    stmt.bindText(1, vehicleId);
    while (stmt.next()) result.push_back(rowToAllocation(db_, stmt));
    return result;
}

std::vector<Allocation> AirportDatabase::listActiveAllocationsForStaff(const std::string& staffId) {
    std::vector<Allocation> result;
    Stmt stmt(db_, R"(
        SELECT a.allocation_id, a.flight_id, a.gate_id, a.vehicle_id, a.window_start, a.window_end, a.status
        FROM allocations a JOIN allocation_staff s ON a.allocation_id = s.allocation_id
        WHERE s.staff_id = ? AND a.status = 'SUCCESS';
    )");
    stmt.bindText(1, staffId);
    while (stmt.next()) result.push_back(rowToAllocation(db_, stmt));
    return result;
}

std::vector<Allocation> AirportDatabase::listAllocationsForFlight(const std::string& flightId) {
    std::vector<Allocation> result;
    Stmt stmt(db_, "SELECT allocation_id, flight_id, gate_id, vehicle_id, window_start, window_end, status "
                   "FROM allocations WHERE flight_id = ?;");
    stmt.bindText(1, flightId);
    while (stmt.next()) result.push_back(rowToAllocation(db_, stmt));
    return result;
}

std::vector<Allocation> AirportDatabase::listAllAllocations() {
    std::vector<Allocation> result;
    Stmt stmt(db_, "SELECT allocation_id, flight_id, gate_id, vehicle_id, window_start, window_end, status "
                   "FROM allocations;");
    while (stmt.next()) result.push_back(rowToAllocation(db_, stmt));
    return result;
}
// ------------------------------------------------------------- Tasks ---

void AirportDatabase::saveGroundTask(const GroundTask& task) {
    Stmt stmt(db_, R"(
        INSERT INTO ground_tasks (task_id, flight_id, task_type, assigned_staff, status, start_time, end_time)
        VALUES (?, ?, ?, ?, ?, ?, ?);
    )");
    stmt.bindText(1, task.getTaskId());
    stmt.bindText(2, task.getFlightId());
    stmt.bindText(3, toString(task.getType()));
    stmt.bindText(4, task.getAssignedStaffId());
    stmt.bindText(5, toString(task.getStatus()));
    stmt.bindText(6, task.getStartTime().toString());
    stmt.bindText(7, task.getEndTime().toString());
    stmt.run();
}

void AirportDatabase::updateTaskStatus(const std::string& taskId, TaskStatus status) {
    Stmt stmt(db_, "UPDATE ground_tasks SET status = ? WHERE task_id = ?;");
    stmt.bindText(1, toString(status));
    stmt.bindText(2, taskId);
    stmt.run();
}

namespace {
GroundTask rowToTask(Stmt& stmt) {
    GroundTask t(stmt.colText(0), stmt.colText(1), parseTaskType(stmt.colText(2)), stmt.colText(3),
                 util::DateTime::fromString(stmt.colText(5)), util::DateTime::fromString(stmt.colText(6)));
    t.updateStatus(parseTaskStatus(stmt.colText(4)));
    return t;
}
}

std::vector<GroundTask> AirportDatabase::listTasksForFlight(const std::string& flightId) {
    std::vector<GroundTask> result;
    Stmt stmt(db_, "SELECT task_id, flight_id, task_type, assigned_staff, status, start_time, end_time "
                   "FROM ground_tasks WHERE flight_id = ?;");
    stmt.bindText(1, flightId);
    while (stmt.next()) result.push_back(rowToTask(stmt));
    return result;
}

std::vector<GroundTask> AirportDatabase::listTasksForStaff(const std::string& staffId) {
    std::vector<GroundTask> result;
    Stmt stmt(db_, "SELECT task_id, flight_id, task_type, assigned_staff, status, start_time, end_time "
                   "FROM ground_tasks WHERE assigned_staff = ?;");
    stmt.bindText(1, staffId);
    while (stmt.next()) result.push_back(rowToTask(stmt));
    return result;
}

std::vector<GroundTask> AirportDatabase::listAllTasks() {
    std::vector<GroundTask> result;
    Stmt stmt(db_, "SELECT task_id, flight_id, task_type, assigned_staff, status, start_time, end_time FROM ground_tasks;");
    while (stmt.next()) result.push_back(rowToTask(stmt));
    return result;
}

// -------------------------------------------------------------- Delays ---

void AirportDatabase::saveDelay(const FlightDelay& delay) {
    Stmt stmt(db_, R"(
        INSERT INTO flight_delays (delay_id, flight_id, reason, delay_minutes, notes, recorded_at)
        VALUES (?, ?, ?, ?, ?, ?);
    )");
    stmt.bindText(1, delay.getDelayId());
    stmt.bindText(2, delay.getFlightId());
    stmt.bindText(3, toString(delay.getReason()));
    stmt.bindInt(4, delay.getDelayMinutes());
    stmt.bindText(5, delay.getNotes());
    stmt.bindText(6, delay.getRecordedAt().toString());
    stmt.run();
}

namespace {
FlightDelay rowToDelay(Stmt& stmt) {
    return FlightDelay(stmt.colText(0), stmt.colText(1), parseDelayReason(stmt.colText(2)),
                        stmt.colInt(3), stmt.colText(4));
}
}

std::vector<FlightDelay> AirportDatabase::listDelaysForFlight(const std::string& flightId) {
    std::vector<FlightDelay> result;
    Stmt stmt(db_, "SELECT delay_id, flight_id, reason, delay_minutes, notes, recorded_at "
                   "FROM flight_delays WHERE flight_id = ?;");
    stmt.bindText(1, flightId);
    while (stmt.next()) result.push_back(rowToDelay(stmt));
    return result;
}

std::vector<FlightDelay> AirportDatabase::listAllDelays() {
    std::vector<FlightDelay> result;
    Stmt stmt(db_, "SELECT delay_id, flight_id, reason, delay_minutes, notes, recorded_at FROM flight_delays;");
    while (stmt.next()) result.push_back(rowToDelay(stmt));
    return result;
}

// ---------------------------------------------------------- Emergency ---

void AirportDatabase::saveEmergency(const EmergencyLanding& emergency) {
    Stmt stmt(db_, R"(
        INSERT INTO emergency_landings (emergency_id, flight_id, reason, declared_at, landing_time, status)
        VALUES (?, ?, ?, ?, ?, ?);
    )");
    stmt.bindText(1, emergency.getEmergencyId());
    stmt.bindText(2, emergency.getFlightId());
    stmt.bindText(3, emergency.getReason());
    stmt.bindText(4, emergency.getDeclaredAt().toString());
    stmt.bindText(5, emergency.getLandingTime().toString());
    stmt.bindText(6, toString(emergency.getStatus()));
    stmt.run();
}

void AirportDatabase::updateEmergency(const EmergencyLanding& emergency) {
    Stmt stmt(db_, "UPDATE emergency_landings SET landing_time = ?, status = ? WHERE emergency_id = ?;");
    stmt.bindText(1, emergency.getLandingTime().toString());
    stmt.bindText(2, toString(emergency.getStatus()));
    stmt.bindText(3, emergency.getEmergencyId());
    stmt.run();
}

std::vector<EmergencyLanding> AirportDatabase::listAllEmergencies() {
    std::vector<EmergencyLanding> result;
    Stmt stmt(db_, "SELECT emergency_id, flight_id, reason, declared_at, landing_time, status FROM emergency_landings;");
    while (stmt.next()) {
        EmergencyLanding e(stmt.colText(0), stmt.colText(1), stmt.colText(2),
                            util::DateTime::fromString(stmt.colText(3)));
        std::string landing = stmt.colText(4);
        if (!landing.empty() && landing != "N/A") e.recordLanding(util::DateTime::fromString(landing));
        e.updateStatus(parseEmergencyStatus(stmt.colText(5)));
        result.push_back(e);
    }
    return result;
}

// ----------------------------------------------------------- Weather ---

void AirportDatabase::logWeather(const WeatherData& data) {
    Stmt stmt(db_, R"(
        INSERT INTO weather_log (temperature_c, wind_speed_kmh, visibility_km, precipitation_mm, condition, observed_at)
        VALUES (?, ?, ?, ?, ?, ?);
    )");
    sqlite3_bind_double(stmt.raw(), 1, data.temperatureCelsius);
    sqlite3_bind_double(stmt.raw(), 2, data.windSpeedKmh);
    sqlite3_bind_double(stmt.raw(), 3, data.visibilityKm);
    sqlite3_bind_double(stmt.raw(), 4, data.precipitationMm);
    stmt.bindText(5, toString(data.condition));
    stmt.bindText(6, data.observedAt.toString());
    stmt.run();
}

// --------------------------------------------------------------- ATC ---

void AirportDatabase::logAtcStatus(const ATCStatus& status) {
    Stmt stmt(db_, R"(
        INSERT INTO atc_log (flight_id, clearance, departure_info, arrival_info, remarks, logged_at)
        VALUES (?, ?, ?, ?, ?, ?);
    )");
    stmt.bindText(1, status.flightId);
    stmt.bindText(2, toString(status.clearance));
    stmt.bindText(3, status.departureInfo);
    stmt.bindText(4, status.arrivalInfo);
    stmt.bindText(5, status.remarks);
    stmt.bindText(6, util::DateTime::now().toString());
    stmt.run();
}

// ----------------------------------------------------------- Reports ---

void AirportDatabase::saveReport(const Report& report) {
    Stmt stmt(db_, R"(
        INSERT INTO reports (report_id, report_type, title, generated_at, content) VALUES (?, ?, ?, ?, ?);
    )");
    stmt.bindText(1, report.getReportId());
    stmt.bindText(2, toString(report.getType()));
    stmt.bindText(3, report.getTitle());
    stmt.bindText(4, report.getGeneratedAt().toString());
    stmt.bindText(5, report.render());
    stmt.run();
}

std::vector<std::pair<std::string, std::string>> AirportDatabase::listReportSummaries() {
    std::vector<std::pair<std::string, std::string>> result;
    Stmt stmt(db_, "SELECT report_id, title FROM reports ORDER BY generated_at DESC;");
    while (stmt.next()) result.emplace_back(stmt.colText(0), stmt.colText(1));
    return result;
}

std::optional<std::string> AirportDatabase::getReportContent(const std::string& reportId) {
    Stmt stmt(db_, "SELECT content FROM reports WHERE report_id = ?;");
    stmt.bindText(1, reportId);
    if (!stmt.next()) return std::nullopt;
    return stmt.colText(0);
}

} // namespace repo
} // namespace agoms
