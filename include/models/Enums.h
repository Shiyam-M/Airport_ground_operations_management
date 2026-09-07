#ifndef AGOMS_ENUMS_H
#define AGOMS_ENUMS_H

#include <string>

namespace agoms {

// ---------------------------------------------------------------------------
// User roles (drives which concrete User subclass an account maps to)
// ---------------------------------------------------------------------------
enum class UserRole {
    ADMINISTRATOR,
    OPERATIONS_MANAGER,
    GROUND_STAFF
};
std::string toString(UserRole role);

// ---------------------------------------------------------------------------
// Flight lifecycle state (see docs/design.md for the full state diagram)
// ---------------------------------------------------------------------------
enum class FlightStatus {
    SCHEDULED,
    APPROACHING,
    ARRIVED,
    EMERGENCY_LANDING,
    GROUND_OPERATIONS,
    BOARDING,
    READY_FOR_DEPARTURE,
    DEPARTED,
    COMPLETED,
    DELAYED,
    CANCELLED
};
std::string toString(FlightStatus status);

// ---------------------------------------------------------------------------
// Resource (Gate / Vehicle / GroundStaffResource) availability status
// ---------------------------------------------------------------------------
enum class ResourceStatus {
    AVAILABLE,
    OCCUPIED,
    MAINTENANCE
};
std::string toString(ResourceStatus status);

enum class ResourceType {
    GATE,
    VEHICLE,
    GROUND_STAFF
};
std::string toString(ResourceType type);

// ---------------------------------------------------------------------------
// Allocation outcome
// ---------------------------------------------------------------------------
enum class AllocationStatus {
    SUCCESS,
    FAILED,
    RELEASED
};
std::string toString(AllocationStatus status);

// ---------------------------------------------------------------------------
// Conflict classification produced by ConflictDetector
// ---------------------------------------------------------------------------
enum class ConflictType {
    NONE,
    GATE_UNAVAILABLE,
    VEHICLE_UNAVAILABLE,
    GROUND_STAFF_UNAVAILABLE,
    TIME_OVERLAP,
    INVALID_FLIGHT_STATE,
    INVALID_RESOURCE
};
std::string toString(ConflictType type);

// ---------------------------------------------------------------------------
// Ground task
// ---------------------------------------------------------------------------
enum class TaskType {
    BAGGAGE_HANDLING,
    AIRCRAFT_CLEANING,
    FUELING,
    CATERING,
    BOARDING_SUPPORT
};
std::string toString(TaskType type);

enum class TaskStatus {
    PENDING,
    ASSIGNED,
    IN_PROGRESS,
    COMPLETED,
    DELAYED
};
std::string toString(TaskStatus status);

// ---------------------------------------------------------------------------
// Delay
// ---------------------------------------------------------------------------
enum class DelayReason {
    WEATHER,
    ATC,
    GROUND_OPERATION,
    RESOURCE_CONFLICT,
    TECHNICAL_ISSUE,
    OTHER
};
std::string toString(DelayReason reason);

// ---------------------------------------------------------------------------
// Emergency landing
// ---------------------------------------------------------------------------
enum class EmergencyStatus {
    DECLARED,
    LANDED,
    GROUND_HANDLING,
    RESOLVED
};
std::string toString(EmergencyStatus status);

// ---------------------------------------------------------------------------
// Weather
// ---------------------------------------------------------------------------
enum class WeatherCondition {
    CLEAR,
    CLOUDY,
    RAIN,
    STORM,
    FOG,
    SNOW
};
std::string toString(WeatherCondition condition);

enum class WeatherAlertLevel {
    NONE,
    ADVISORY,
    WARNING,
    SEVERE
};
std::string toString(WeatherAlertLevel level);

// ---------------------------------------------------------------------------
// ATC
// ---------------------------------------------------------------------------
enum class ATCClearance {
    PENDING,
    GRANTED,
    DENIED,
    HOLD
};
std::string toString(ATCClearance clearance);

// ---------------------------------------------------------------------------
// Reporting
// ---------------------------------------------------------------------------
enum class ReportType {
    FLIGHT_STATUS,
    RESOURCE_UTILIZATION,
    ALLOCATION,
    DELAY,
    EMERGENCY,
    GROUND_OPERATION
};
std::string toString(ReportType type);

} // namespace agoms

#endif // AGOMS_ENUMS_H
