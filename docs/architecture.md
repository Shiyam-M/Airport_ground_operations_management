# AGOMS — Architecture

## Layered architecture

```
Presentation Layer        controllers/ConsoleUI
        |
        v
Service / Business Logic  services/*  (FlightService, ResourceService,
Layer                      AllocationService, ConflictDetector, ATCService,
                            WeatherService, FlightMonitor, DelayService,
                            EmergencyService, GroundOperationsService,
                            ReportService, UserService)
        |
        v
Domain / Model Layer       models/*  (Flight, Resource, Allocation,
                            GroundTask, FlightDelay, EmergencyLanding,
                            WeatherData, ATCStatus, Report, User, ...)
        |
        v
Repository / Database      repositories/AirportDatabase (SQLite-backed)
Layer
```

Each layer only depends on the layer directly below it:

- `ConsoleUI` calls into the service layer only. It never issues SQL or
  touches `AirportDatabase` directly for business operations, and it never
  contains business rules (e.g. it does not decide whether an allocation is
  valid — it just displays whatever `AllocationService` returns).
- The service layer contains all business rules and orchestration. Services
  depend on `AirportDatabase` for persistence and on domain model classes,
  but never on the UI.
- The domain model layer (`models/*`) is UI- and persistence-agnostic. Model
  classes enforce their own invariants (e.g. `Flight::transitionTo()`
  refuses illegal state transitions) but know nothing about SQL or the
  console.
- `AirportDatabase` is the single Repository-pattern gateway to SQLite. It
  is the only file in the project that constructs SQL strings.

## Component / composition view

```
UI (ConsoleUI)
 |
 +-- UserService
 +-- FlightService
 +-- ResourceService
 +-- AllocationService ---- ConflictDetector
 +-- ATCService
 +-- WeatherService
 +-- FlightMonitor -------- ATCService
 |                    \---- WeatherService
 |                    \---- FlightService
 +-- DelayService --------- FlightService
 |                    \---- AllocationService
 |                    \---- ConflictDetector
 +-- EmergencyService ----- FlightService
 +-- GroundOperationsService
 +-- ReportService
 |
 v
AirportDatabase (repositories/)
 |
 v
SQLite (via third_party/sqlite3/sqlite3_min.h + libsqlite3)
```

## Why a hand-written `sqlite3_min.h`?

This project links against the real, system-provided SQLite3 shared library
(`libsqlite3.so` / `libsqlite3.so.0`), which ships as a base dependency on
virtually every Linux distribution. Some minimal build environments,
however, only have the *runtime* library installed and not the
`libsqlite3-dev` package that provides the official `sqlite3.h` header
(and may have no package-manager/internet access to install it).

The SQLite C API/ABI has been stable for well over a decade, so
`third_party/sqlite3/sqlite3_min.h` hand-declares only the subset of
functions `AirportDatabase` actually calls (`sqlite3_open`,
`sqlite3_prepare_v2`, `sqlite3_bind_*`, `sqlite3_step`, `sqlite3_column_*`,
etc.) with signatures copied verbatim from the official API. This gives a
**real** SQLite-backed persistence layer — not a mock or a hand-rolled flat
file format — while remaining buildable in restricted environments.

If the official `sqlite3.h` **is** available on your machine, nothing needs
to change: `CMakeLists.txt` still locates and links `libsqlite3`
transparently, and you may swap in `#include <sqlite3.h>` if you prefer;
the function signatures are identical.

## Design pattern usage

| Pattern    | Where | Why |
|------------|-------|-----|
| Repository | `AirportDatabase` | Isolates all SQL from the rest of the codebase behind typed methods (`saveFlight`, `listGates`, `listActiveAllocationsForGate`, ...). |
| State (via transition table) | `Flight::transitionTo` / `Flight::isTransitionAllowed` | The flight lifecycle has ~20 legal edges. A static adjacency table is easier to read, test, and audit than 11 separate `State` subclasses, and is still trivially unit-testable (see `tests/test_flight.cpp`). We deliberately avoided a full class-per-state State Pattern here per the brief's "do not over-engineer it" guidance. |
| Strategy-like composition | `FlightMonitor` composes `ATCService` + `WeatherService` + `FlightService` | Each concern (ATC, weather, flight identity) is independently testable and swappable. |
| Facade / Service Layer | `services/*` | Each service presents a small, purpose-specific facade over `AirportDatabase` + domain objects, matching the UML module boundaries (Flight Management, Resource Management, Allocation Management, Conflict Detection, etc). |

We intentionally did **not** use a Factory for `User`/`Resource` creation:
with only 3 user subclasses and 3 resource subclasses, direct construction
in `AirportDatabase`'s row-mapping code and in the service layer is simpler
and equally clear.

## Threading

The core allocation, conflict-detection, and persistence operations are
entirely synchronous — there is no hidden concurrency in the request path,
which keeps the allocation workflow easy to reason about and test
deterministically (see `tests/test_allocation.cpp`). No background threads
are used anywhere in this implementation; `WeatherService`/`ATCService` are
called synchronously, on demand, from the console UI (menu options 10/11)
and from `FlightMonitor`. The codebase is structured so that periodic
background polling (e.g. a thread that calls `FlightMonitor::monitor()`
every N seconds for all in-progress flights) could be added later without
touching any business logic — it would simply be a new thin driver that
calls the existing, already-thread-safe-by-virtue-of-being-synchronous
service methods on a timer.
