# AGOMS — Design Notes

## Flight lifecycle (state diagram)

```
Scheduled
    |
    v
Approaching ------------------> Cancelled
    |        \
    |         \--> EmergencyLanding --> GroundOperations
    v
Arrived
    |
    v
GroundOperations <--------------------\
    |            \                     \
    v             v                     |
Boarding       Delayed ------------------
    |            |  \
    v             |   \--> Cancelled
ReadyForDeparture |
    |             |
    v             |
Departed <---------
    |
    v
Completed
```

Legal transitions (implemented as a static edge table in
`Flight::isTransitionAllowed`, `src/models/Flight.cpp`):

| From | To |
|---|---|
| SCHEDULED | APPROACHING, CANCELLED |
| APPROACHING | ARRIVED, EMERGENCY_LANDING, CANCELLED |
| EMERGENCY_LANDING | GROUND_OPERATIONS |
| ARRIVED | GROUND_OPERATIONS |
| GROUND_OPERATIONS | BOARDING, DELAYED |
| BOARDING | READY_FOR_DEPARTURE, DELAYED |
| READY_FOR_DEPARTURE | DEPARTED, DELAYED |
| DEPARTED | COMPLETED |
| DELAYED | GROUND_OPERATIONS, BOARDING, READY_FOR_DEPARTURE, CANCELLED |

`DelayService::applyDelay()` transitions the flight to `DELAYED` and shifts
its `FlightSchedule` estimated times forward; the Operations Manager (via
`ConsoleUI` -> `FlightService::transitionFlightStatus`) then resumes the
flight back into whichever operational state is appropriate once the
ground situation is clear (this mirrors "Delayed -> appropriate operational
state after schedule update" from the specification — the implementation
deliberately leaves the *choice* of resume-state to the human Operations
Manager rather than guessing it automatically).

Any transition not in the table above throws
`InvalidStateTransitionException`, which `ConsoleUI` catches and reports as
a friendly error instead of crashing (see `tests/test_flight.cpp` for
coverage of both legal and illegal transitions).

## Resource allocation workflow

```
Operations Manager
        |
        v
ConsoleUI (handleAllocateResources)
        |
        v
AllocationService::allocate(AllocationRequest)
        |
        v
  1. Look up the flight in AirportDatabase; validate it exists.
  2. Validate the flight's current status permits new allocations
     (SCHEDULED / APPROACHING / ARRIVED / EMERGENCY_LANDING /
      GROUND_OPERATIONS / DELAYED).
  3-6. ConflictDetector::checkConflict(request):
       - validates gate/vehicle/staff IDs exist
       - checks each resource isn't in MAINTENANCE
       - checks each resource has no overlapping active allocation
         for the requested time window (queried from AirportDatabase)
        |
        v
  7. If ConflictResult.hasConflict() == false:
       - mark gate/vehicle/staff OCCUPIED in AirportDatabase
       - persist a new Allocation record
       - return AllocationResult::success(allocationId)
     else:
       - mutate nothing
       - return AllocationResult::failure(conflictResult)
```

`AllocationService::release(allocationId)` reverses step 7: it sets the
gate/vehicle/staff back to `AVAILABLE` and marks the `Allocation` as
`RELEASED`. This is exposed in the console UI under
**"4. Allocate Resources" -> "2. Release allocation"**.

## Conflict detection

`ConflictDetector::checkConflict()` (in `services/ConflictDetector.cpp`)
returns a `ConflictResult` with:

- `hasConflict()` — `true`/`false`
- `getConflictType()` — one of `NONE`, `GATE_UNAVAILABLE`,
  `VEHICLE_UNAVAILABLE`, `GROUND_STAFF_UNAVAILABLE`, `TIME_OVERLAP` (folded
  into the specific resource type above, since an overlap is always
  reported against the resource it overlaps on), `INVALID_FLIGHT_STATE`,
  `INVALID_RESOURCE`
- `getDetails()` — a human-readable explanation, e.g.
  `"Gate G01 is already allocated to FL001 for 2026-08-26 09:00 - 2026-08-26 10:30."`

Two time windows `[aStart, aEnd)` and `[bStart, bEnd)` are considered
overlapping via `util::timeWindowsOverlap`, i.e. `aStart < bEnd && bStart <
aEnd`.

## Resource status lifecycle

Every `Resource` subclass (`Gate`, `Vehicle`, `GroundStaffResource`) carries
a `ResourceStatus`: `AVAILABLE`, `OCCUPIED`, `MAINTENANCE`.

- `AllocationService::allocate()` is the only code path that flips
  `AVAILABLE -> OCCUPIED` as a *side effect* of a successful allocation.
- `AllocationService::release()` is the only code path that flips
  `OCCUPIED -> AVAILABLE` as a side effect of a release.
- An administrator can independently set a resource to `MAINTENANCE` (or
  back) via `ResourceService`, which `ConflictDetector` then honors as an
  unconditional unavailability regardless of the time window requested.

## Delay handling workflow

Implemented in `services/DelayService.cpp`, matching the specification
step-by-step:

1. Record delay reason + duration (`FlightDelay` persisted).
2. Flight status -> `DELAYED`.
3. `FlightSchedule` estimated arrival/departure shifted forward by the
   delay duration.
4-6. Every currently `SUCCESS` allocation for the flight is re-checked
   against `ConflictDetector` using the *new* estimated time window; any
   allocation that would now conflict with someone else's resource is
   surfaced as a warning string in `DelayOutcome::resourceWarnings` so the
   Operations Manager can manually release/reallocate as needed (the
   system intentionally does not silently swap in a *different* physical
   resource on the manager's behalf).
7. `ConsoleUI` displays the updated schedule and any warnings immediately
   after the call returns.

## Emergency landing workflow

Implemented in `services/EmergencyService.cpp`:

1. `declareEmergency(flightId, reason)`: transitions the flight
   `APPROACHING -> EMERGENCY_LANDING` and persists an `EmergencyLanding`
   record with status `DECLARED`.
2. `recordLanding(emergencyId)`: records the landing timestamp, status ->
   `LANDED`.
3. `beginGroundHandling(flightId, emergencyId)`: transitions the flight
   `EMERGENCY_LANDING -> GROUND_OPERATIONS`, emergency status ->
   `GROUND_HANDLING`.
4. `resolveEmergency(emergencyId)`: emergency status -> `RESOLVED`
   (informational close-out; the flight itself continues through its
   normal ground-operations lifecycle from here).

## ATC & Weather integration

Both `ATCService` and `WeatherService` are explicitly documented, in-code,
as **simulated/mock integrations** per the specification ("this is an
academic simulation... do not require an actual external API"):

- `WeatherService::getCurrentWeather()` produces a bounded pseudo-random
  snapshot (temperature/wind/visibility/precipitation/condition) seeded
  from the system clock, and logs it to the `weather_log` table.
  `checkWeatherAlert()` derives a `WeatherAlert` (`NONE` / `ADVISORY` /
  `WARNING` / `SEVERE`) from thresholds on that data.
- `ATCService::getFlightStatus()` deterministically derives a plausible
  `ATCClearance` / departure / arrival description from the flight's
  *current* `FlightStatus`, and logs it to the `atc_log` table.

Both expose the exact method names requested in the specification
(`getFlightStatus`, `getATCClearance`, `getDepartureStatus`,
`getCurrentWeather`, `checkWeatherAlert`), so a real integration could be
substituted later without changing any caller.

`FlightMonitor` composes `ATCService` + `WeatherService` + `FlightService`
to produce a `MonitoringSnapshot` (flight status, ATC status, weather,
weather alert, and a list of detected operational issues) — this is what
powers console menu option **6. View Flight Status**.
