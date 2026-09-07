# Airport Ground Operations Management System (AGOMS)

A C++17 console application for managing airport ground resources and
flight operations: flights, gates, vehicles, ground staff, resource
allocation with conflict detection, ground operations, flight delays,
emergency landings, and simulated ATC/Weather integration — backed by a
real SQLite persistence layer.

## 1. Project Overview

AGOMS lets an **Operations Manager** allocate gates, vehicles and ground
staff to flights while automatically detecting and rejecting conflicting
allocations; track flights through a well-defined lifecycle state machine;
handle delays and emergency landings; and generate operational reports. An
**Airport Administrator** manages the underlying master data (flights,
gates, vehicles, ground staff, user accounts). **Ground Staff** log in to
see their assigned tasks and update task status. The system also exposes
simulated **ATC** and **Weather** integrations that feed into flight
monitoring, delay handling and emergency response.

## 2. Objective

Build a layered, testable, UML-faithful C++ system that demonstrates:
resource-conflict-safe allocation, a proper flight-state lifecycle, clean
separation of presentation / business logic / domain / persistence, and a
real (not mocked) relational persistence layer, entirely from the console.

## 3. Scope

In scope: flight & resource master data management, allocation with
conflict detection, ground task tracking, delay handling, emergency
landing handling, simulated ATC/Weather integration, reporting, and a
console UI. Out of scope (explicitly, per the assignment brief): a GUI,
a real external ATC/Weather API, and multithreaded background schedulers
(the system is fully synchronous and correct without threads; see
`docs/architecture.md` §Threading for how background polling could be
layered on top later without touching business logic).

## 4. Features

- Role-based accounts: Airport Administrator, Operations Manager, Ground Staff
- Flight master data + an 11-state lifecycle state machine with a validated
  transition table (`Scheduled → Approaching → Arrived → GroundOperations →
  Boarding → ReadyForDeparture → Departed → Completed`, plus
  `EmergencyLanding`, `Delayed`, `Cancelled` side-paths)
- Gate / Vehicle / GroundStaffResource master data with
  `AVAILABLE / OCCUPIED / MAINTENANCE` status
- Resource allocation with full conflict detection (gate/vehicle/staff
  availability + time-window overlap) before any resource is committed
- Resource release (returns resources to `AVAILABLE`)
- Ground task management (baggage handling, cleaning, fueling, catering,
  boarding support) with `PENDING/ASSIGNED/IN_PROGRESS/COMPLETED/DELAYED`
  status, updatable by Ground Staff
- Flight delay handling: records reason & duration, updates flight status
  and schedule, and re-validates previously allocated resources against the
  new time window
- Emergency landing handling: declare → record landing → begin ground
  handling → resolve
- Simulated ATC integration (`getFlightStatus`, `getATCClearance`,
  `getDepartureStatus`)
- Simulated Weather integration (`getCurrentWeather`, `checkWeatherAlert`)
- Flight monitoring that composes ATC + Weather + Flight state into one
  operational snapshot
- Six report types: flight status, resource utilization, allocation, delay,
  emergency, ground operation — generated and persisted for later recall
- Real SQLite persistence (not an in-memory-only mock)
- 27 unit tests covering flight/resource creation, successful allocation,
  gate/vehicle/staff conflicts, release, state transitions, delay,
  emergency landing, and database persistence-across-reopen

## 5. Architecture

```
Presentation Layer   -> controllers/ConsoleUI
Service Layer        -> services/* (FlightService, ResourceService,
                         AllocationService, ConflictDetector, ATCService,
                         WeatherService, FlightMonitor, DelayService,
                         EmergencyService, GroundOperationsService,
                         ReportService, UserService)
Domain Model Layer    -> models/* (Flight, Resource, Allocation, GroundTask,
                         FlightDelay, EmergencyLanding, WeatherData,
                         ATCStatus, Report, User hierarchy, ...)
Repository Layer      -> repositories/AirportDatabase (SQLite-backed)
```

Full details, the design-pattern rationale, and a discussion of the
project's SQLite linking approach are in **`docs/architecture.md`**.
Complete workflow walkthroughs (allocation, conflict detection, delay,
emergency landing, flight lifecycle) are in **`docs/design.md`**.

## 6. Module ↔ Class Map

| Module (spec §3) | Key classes |
|---|---|
| A. User Management | `User`, `AirportAdministrator`, `OperationsManager`, `GroundStaff`, `UserService` |
| B. Flight Management | `Flight`, `FlightSchedule`, `FlightService` |
| C. Resource Management | `Resource`, `Gate`, `Vehicle`, `GroundStaffResource`, `ResourceService` |
| D. Allocation Management | `Allocation`, `AllocationRequest`, `AllocationResult`, `AllocationService` |
| E. Conflict Detection | `ConflictDetector`, `ConflictResult` |
| F. Ground Operations Management | `GroundTask`, `GroundOperationsService` |
| G. Flight Monitoring | `FlightMonitor`, `MonitoringSnapshot` |
| H. Delay Management | `FlightDelay`, `DelayService`, `DelayOutcome` |
| I. Emergency Landing Management | `EmergencyLanding`, `EmergencyService` |
| J. ATC Integration | `ATCStatus`, `ATCService` |
| K. Weather Integration | `WeatherData`, `WeatherAlert`, `WeatherService` |
| L. Reporting | `Report`, `ReportService` |
| M. Database/Persistence | `AirportDatabase` |

**Note on `GroundStaff` vs `GroundStaffResource`:** the specification lists
`GroundStaff` both as a `User` subclass (an actor who logs in) and
implicitly as an allocatable resource. This implementation keeps those as
two distinct classes — `models/GroundStaff.h` (a login identity, subclass
of `User`) and `models/GroundStaffResource.h` (an allocatable crew
resource, subclass of `Resource`) — linked via
`GroundStaff::getLinkedResourceId()`. This avoids conflating "who can log
in" with "what can be scheduled," which would otherwise force `User` to
carry `ResourceStatus` and allocation-window fields that make no sense for
an Administrator or Operations Manager account.

## 7. Database Design

SQLite, accessed through `repositories/AirportDatabase`. Full DDL is in
[`database/schema.sql`](database/schema.sql) (also executed
programmatically at startup via `AirportDatabase::initializeSchema()`, so
the file and the code are guaranteed to match). Key tables:

`users`, `flights`, `flight_schedules`, `gates`, `vehicles`,
`ground_staff_resources`, `allocations` (+ `allocation_staff` junction
table for the many ground-staff-per-allocation relationship),
`ground_tasks`, `flight_delays`, `emergency_landings`, `weather_log`,
`atc_log`, `reports`.

All queries are parameterized (`sqlite3_bind_*`) — no string-concatenated
SQL anywhere in the codebase.

**Why a hand-written `sqlite3_min.h` instead of `#include <sqlite3.h>`?**
See `docs/architecture.md` — in short: this project links against the real
system `libsqlite3` shared library, but declares the small subset of the
C API it needs by hand in `third_party/sqlite3/sqlite3_min.h`, so that it
still builds in environments that have the SQLite runtime but not the
`libsqlite3-dev` header package. If your machine has `libsqlite3-dev`
installed, the build works identically either way.

## 8. Build Instructions

**Requirements:** a C++17 compiler (g++ 9+ / clang 10+), CMake 3.16+, and
the SQLite3 runtime library (`libsqlite3-0` or equivalent — present on
almost all Linux systems by default; install `libsqlite3-dev` too if you'd
rather use the official `sqlite3.h`, which also works unmodified).

```bash
cmake -S . -B build
cmake --build build -j4
```

This produces:
- `build/agoms_app` — the main console application
- `build/agoms_tests` — the unit test suite

If CMake reports it cannot find `libsqlite3` on an unusual system, install
the runtime library, e.g. on Debian/Ubuntu:
```bash
sudo apt-get install libsqlite3-0        # runtime only, sufficient to build this project
# or
sudo apt-get install libsqlite3-dev      # official headers, also works
```

**Manual build (no CMake required)** — every source file compiles cleanly
with a single g++ invocation, which is how this project was verified in an
offline sandbox with no CMake available:
```bash
g++ -std=c++17 -Iinclude -Ithird_party/sqlite3 \
    $(find src -name '*.cpp') main.cpp \
    -o agoms_app -lsqlite3
```
(If `-lsqlite3` isn't found because only the runtime `.so.0` exists without
a `.so` symlink, link the file directly, e.g.
`/usr/lib/x86_64-linux-gnu/libsqlite3.so.0` — this is exactly what
`CMakeLists.txt` does automatically via its `find_library` fallback.)

## 9. Run Instructions

```bash
./build/agoms_app                # creates/opens ./agoms.db in the current directory
./build/agoms_app custom_path.db # or specify a database file explicitly
```

On first run against a fresh/empty database, the app automatically creates
the schema and seeds sample data (see §12 below). On subsequent runs it
reuses the existing data.

## 10. Test Instructions

```bash
cmake --build build --target agoms_tests
./build/agoms_tests
```

A custom, dependency-free test framework (`tests/TestFramework.h`) is used
instead of GoogleTest, specifically so the project builds in fully offline
environments (no `FetchContent`/network access required). If GoogleTest is
available in your environment, the `TEST(name) { ... }` /
`ASSERT_EQ/ASSERT_TRUE/ASSERT_THROWS` style used throughout `tests/*.cpp`
maps directly onto `TEST(Suite, Name) { ... }` / `EXPECT_*`/`ASSERT_*` if
you'd prefer to port it.

**Latest verified run: 27/27 tests passing** (see §13 for full output).

## 11. Sample Login Credentials

Seeded automatically on first run:

| Username | Password | Role |
|---|---|---|
| `admin` | `admin123` | Airport Administrator |
| `opsmgr` | `ops123` | Operations Manager |
| `staff1` | `staff123` | Ground Staff (linked to resource `GS01`) |
| `staff2` | `staff123` | Ground Staff (linked to resource `GS02`) |

## 12. Sample Data

Seeded on first run against an empty database (also documented in
[`database/seed.sql`](database/seed.sql)):

- Flights: `FL001` (AI101, DEL→BOM), `FL002` (AI202, BOM→BLR), `FL003`
  (AI303, BLR→MAA), each with a schedule on 2026-08-26
- Gates: `G01`, `G02` (Terminal A), `G03` (Terminal B) — all `AVAILABLE`
- Vehicles: `V01` (Baggage Tractor), `V02` (Fuel Truck) — all `AVAILABLE`
- Ground staff resources: `GS01`..`GS04` — all `AVAILABLE`

## 13. Sample Workflow & Verified Test Results

### 13.1 Allocation success + conflict detection (the required demo scenario)

Logged in as `opsmgr`, from the Main Menu: **4. Allocate Resources → 1. New
allocation**.

**Request 1** — Flight `FL001`, Gate `G01`, Vehicle `V01`, Staff
`GS01,GS02`, window `2026-08-26 09:00`–`2026-08-26 10:30`:

```
Allocation Succeeded. Allocation ID: AL0001
Gate G01 -> OCCUPIED
Vehicle V01 -> OCCUPIED
Staff GS01 -> OCCUPIED
Staff GS02 -> OCCUPIED
```

**Request 2** — Flight `FL002`, Gate `G01` (same gate, overlapping window
`09:15`–`10:00`):

```
Allocation Failed
Conflict: Gate G01 is already allocated to FL001 for 2026-08-26 09:00 - 2026-08-26 10:30.
```

**Releasing** `AL0001` (Main Menu → 4 → 2 → `AL0001`):

```
Allocation AL0001 released. Gate, vehicle and ground staff are now AVAILABLE again.
```

Listing gates afterward confirms `G01` is `AVAILABLE` again, and a new
flight can now successfully allocate it for the same window.

*(This exact sequence is also covered automatically by
`Allocation_SuccessfulAllocationOccupiesResourcesAndPersistsRecord`,
`Allocation_OverlappingGateRequestFails`, and
`Allocation_ReleaseReturnsResourcesToAvailable` in `tests/test_allocation.cpp`.)*

### 13.2 Full flight lifecycle

Verified end-to-end (Main Menu → 2 → 3, repeated):
`SCHEDULED → APPROACHING → ARRIVED → GROUND_OPERATIONS → (delay applied,
→ DELAYED) → GROUND_OPERATIONS → BOARDING → READY_FOR_DEPARTURE →
DEPARTED → COMPLETED`, with each illegal transition attempt (e.g.
`SCHEDULED → DEPARTED`) correctly rejected with a clear error message
rather than crashing.

### 13.3 Unit test results (latest run)

```
[ PASS ] ResourceCreation_GateVehicleStaffAllPersist
[ PASS ] ResourceCreation_DuplicateGateRejected
[ PASS ] ResourceService_SetStatusToMaintenance
[ PASS ] ResourceService_UnknownGateThrows
[ PASS ] Allocation_SuccessfulAllocationOccupiesResourcesAndPersistsRecord
[ PASS ] Allocation_OverlappingGateRequestFails
[ PASS ] Allocation_OverlappingVehicleRequestFails
[ PASS ] Allocation_OverlappingGroundStaffRequestFails
[ PASS ] Allocation_NonOverlappingWindowsOnSameGateSucceed
[ PASS ] Allocation_ReleaseReturnsResourcesToAvailable
[ PASS ] Allocation_ReleasingAlreadyReleasedAllocationThrows
[ PASS ] Allocation_UnknownFlightFails
[ PASS ] Delay_ApplyDelayUpdatesStatusAndSchedule
[ PASS ] Delay_NonPositiveDurationRejected
[ PASS ] Delay_RecordedInHistory
[ PASS ] Emergency_DeclareTransitionsFlightAndRecordsDetails
[ PASS ] Emergency_FullWorkflowReachesGroundHandling
[ PASS ] Emergency_CannotDeclareFromScheduledState
[ PASS ] FlightCreation_PersistsAndIsRetrievable
[ PASS ] FlightCreation_DuplicateIdRejected
[ PASS ] FlightStateTransition_ValidChainSucceeds
[ PASS ] FlightStateTransition_IllegalTransitionRejected
[ PASS ] FlightStateTransition_EmergencyPathAllowed
[ PASS ] FlightService_GetFlightThrowsWhenNotFound
[ PASS ] Database_DataSurvivesReopen
[ PASS ] Database_SeedDataIsIdempotent
[ PASS ] Database_UserAuthenticationRoundTrips

Total: 27 | Passed: 27 | Failed: 0
```

## 14. Resource Allocation Flow

See `docs/design.md` §"Resource allocation workflow" for the full
step-by-step trace matching spec §8
(`UI → AllocationService → validate flight → ConflictDetector →
AirportDatabase → AllocationResult`).

## 15. Conflict Handling

See `docs/design.md` §"Conflict detection". Summary: `ConflictDetector`
validates flight/resource IDs exist, checks each requested resource isn't
in `MAINTENANCE`, and checks each requested resource has no other
`SUCCESS`-status allocation whose time window overlaps the request. Any one
conflict aborts the whole allocation atomically (no partial allocation is
ever persisted).

## 16. Flight State Lifecycle

See §4 above and `docs/design.md` §"Flight lifecycle" for the full
transition table and rationale.

## 17. ATC Integration

`ATCService` (simulated) exposes `getFlightStatus`, `getATCClearance`,
`getDepartureStatus`, `getArrivalStatus`. Every call is also logged to the
`atc_log` table. See `docs/design.md` §"ATC & Weather integration".

## 18. Weather Integration

`WeatherService` (simulated) exposes `getCurrentWeather` (bounded
pseudo-random snapshot, logged to `weather_log`) and `checkWeatherAlert`
(derives `NONE`/`ADVISORY`/`WARNING`/`SEVERE` from wind/visibility/
precipitation/condition thresholds). See `docs/design.md`.

## 19. Future Enhancements

- Automatic reallocation suggestions (rather than warnings only) when a
  delay invalidates a previously allocated resource
- A background monitoring thread that periodically calls
  `FlightMonitor::monitor()` for all in-flight/ground-active flights and
  proactively raises alerts, rather than requiring a manual menu check
  (the architecture already supports this without any service-layer
  changes — see `docs/architecture.md` §Threading)
- Password hashing upgraded from the current demo-grade hash to
  bcrypt/argon2 with per-user salt for any real deployment
- A real external ATC/Weather API integration behind the existing
  `ATCService`/`WeatherService` interfaces
- A web or GUI front-end reusing the existing service layer unchanged

## 20. Project Structure

```
AirportGroundOperations/
├── CMakeLists.txt
├── README.md
├── LICENSE
├── main.cpp
├── include/
│   ├── models/         (Enums, User hierarchy, Flight, Resource hierarchy,
│   │                     Allocation*, GroundTask, FlightDelay,
│   │                     EmergencyLanding, WeatherData/Alert, ATCStatus,
│   │                     Report, ConflictResult)
│   ├── services/        (UserService, FlightService, ResourceService,
│   │                     ConflictDetector, AllocationService, ATCService,
│   │                     WeatherService, FlightMonitor, DelayService,
│   │                     EmergencyService, GroundOperationsService,
│   │                     ReportService)
│   ├── repositories/     (AirportDatabase)
│   ├── controllers/      (ConsoleUI)
│   └── utils/            (DateTime, Exceptions)
├── src/                 (mirrors include/, plus the .cpp implementations)
├── third_party/sqlite3/  (sqlite3_min.h — see docs/architecture.md)
├── database/
│   ├── schema.sql
│   └── seed.sql
├── tests/
│   ├── TestFramework.h
│   ├── test_main.cpp
│   ├── test_flight.cpp
│   ├── test_resources.cpp
│   ├── test_allocation.cpp
│   ├── test_delay_emergency.cpp
│   └── test_database.cpp
└── docs/
    ├── architecture.md
    └── design.md
```
