#include "models/Enums.h"

namespace agoms {

std::string toString(UserRole role) {
    switch (role) {
        case UserRole::ADMINISTRATOR:      return "ADMINISTRATOR";
        case UserRole::OPERATIONS_MANAGER: return "OPERATIONS_MANAGER";
        case UserRole::GROUND_STAFF:       return "GROUND_STAFF";
    }
    return "UNKNOWN";
}

std::string toString(FlightStatus status) {
    switch (status) {
        case FlightStatus::SCHEDULED:            return "SCHEDULED";
        case FlightStatus::APPROACHING:          return "APPROACHING";
        case FlightStatus::ARRIVED:              return "ARRIVED";
        case FlightStatus::EMERGENCY_LANDING:    return "EMERGENCY_LANDING";
        case FlightStatus::GROUND_OPERATIONS:    return "GROUND_OPERATIONS";
        case FlightStatus::BOARDING:             return "BOARDING";
        case FlightStatus::READY_FOR_DEPARTURE:  return "READY_FOR_DEPARTURE";
        case FlightStatus::DEPARTED:             return "DEPARTED";
        case FlightStatus::COMPLETED:            return "COMPLETED";
        case FlightStatus::DELAYED:              return "DELAYED";
        case FlightStatus::CANCELLED:            return "CANCELLED";
    }
    return "UNKNOWN";
}

std::string toString(ResourceStatus status) {
    switch (status) {
        case ResourceStatus::AVAILABLE:   return "AVAILABLE";
        case ResourceStatus::OCCUPIED:    return "OCCUPIED";
        case ResourceStatus::MAINTENANCE: return "MAINTENANCE";
    }
    return "UNKNOWN";
}

std::string toString(ResourceType type) {
    switch (type) {
        case ResourceType::GATE:         return "GATE";
        case ResourceType::VEHICLE:      return "VEHICLE";
        case ResourceType::GROUND_STAFF: return "GROUND_STAFF";
    }
    return "UNKNOWN";
}

std::string toString(AllocationStatus status) {
    switch (status) {
        case AllocationStatus::SUCCESS:  return "SUCCESS";
        case AllocationStatus::FAILED:   return "FAILED";
        case AllocationStatus::RELEASED: return "RELEASED";
    }
    return "UNKNOWN";
}

std::string toString(ConflictType type) {
    switch (type) {
        case ConflictType::NONE:                     return "NONE";
        case ConflictType::GATE_UNAVAILABLE:         return "GATE_UNAVAILABLE";
        case ConflictType::VEHICLE_UNAVAILABLE:      return "VEHICLE_UNAVAILABLE";
        case ConflictType::GROUND_STAFF_UNAVAILABLE: return "GROUND_STAFF_UNAVAILABLE";
        case ConflictType::TIME_OVERLAP:             return "TIME_OVERLAP";
        case ConflictType::INVALID_FLIGHT_STATE:     return "INVALID_FLIGHT_STATE";
        case ConflictType::INVALID_RESOURCE:         return "INVALID_RESOURCE";
    }
    return "UNKNOWN";
}

std::string toString(TaskType type) {
    switch (type) {
        case TaskType::BAGGAGE_HANDLING:  return "BAGGAGE_HANDLING";
        case TaskType::AIRCRAFT_CLEANING: return "AIRCRAFT_CLEANING";
        case TaskType::FUELING:           return "FUELING";
        case TaskType::CATERING:          return "CATERING";
        case TaskType::BOARDING_SUPPORT:  return "BOARDING_SUPPORT";
    }
    return "UNKNOWN";
}

std::string toString(TaskStatus status) {
    switch (status) {
        case TaskStatus::PENDING:     return "PENDING";
        case TaskStatus::ASSIGNED:    return "ASSIGNED";
        case TaskStatus::IN_PROGRESS: return "IN_PROGRESS";
        case TaskStatus::COMPLETED:   return "COMPLETED";
        case TaskStatus::DELAYED:     return "DELAYED";
    }
    return "UNKNOWN";
}

std::string toString(DelayReason reason) {
    switch (reason) {
        case DelayReason::WEATHER:           return "WEATHER";
        case DelayReason::ATC:               return "ATC";
        case DelayReason::GROUND_OPERATION:  return "GROUND_OPERATION";
        case DelayReason::RESOURCE_CONFLICT: return "RESOURCE_CONFLICT";
        case DelayReason::TECHNICAL_ISSUE:   return "TECHNICAL_ISSUE";
        case DelayReason::OTHER:             return "OTHER";
    }
    return "UNKNOWN";
}

std::string toString(EmergencyStatus status) {
    switch (status) {
        case EmergencyStatus::DECLARED:       return "DECLARED";
        case EmergencyStatus::LANDED:         return "LANDED";
        case EmergencyStatus::GROUND_HANDLING: return "GROUND_HANDLING";
        case EmergencyStatus::RESOLVED:       return "RESOLVED";
    }
    return "UNKNOWN";
}

std::string toString(WeatherCondition condition) {
    switch (condition) {
        case WeatherCondition::CLEAR:  return "CLEAR";
        case WeatherCondition::CLOUDY: return "CLOUDY";
        case WeatherCondition::RAIN:   return "RAIN";
        case WeatherCondition::STORM:  return "STORM";
        case WeatherCondition::FOG:    return "FOG";
        case WeatherCondition::SNOW:   return "SNOW";
    }
    return "UNKNOWN";
}

std::string toString(WeatherAlertLevel level) {
    switch (level) {
        case WeatherAlertLevel::NONE:     return "NONE";
        case WeatherAlertLevel::ADVISORY: return "ADVISORY";
        case WeatherAlertLevel::WARNING:  return "WARNING";
        case WeatherAlertLevel::SEVERE:   return "SEVERE";
    }
    return "UNKNOWN";
}

std::string toString(ATCClearance clearance) {
    switch (clearance) {
        case ATCClearance::PENDING: return "PENDING";
        case ATCClearance::GRANTED: return "GRANTED";
        case ATCClearance::DENIED:  return "DENIED";
        case ATCClearance::HOLD:    return "HOLD";
    }
    return "UNKNOWN";
}

std::string toString(ReportType type) {
    switch (type) {
        case ReportType::FLIGHT_STATUS:         return "FLIGHT_STATUS";
        case ReportType::RESOURCE_UTILIZATION:  return "RESOURCE_UTILIZATION";
        case ReportType::ALLOCATION:            return "ALLOCATION";
        case ReportType::DELAY:                 return "DELAY";
        case ReportType::EMERGENCY:             return "EMERGENCY";
        case ReportType::GROUND_OPERATION:      return "GROUND_OPERATION";
    }
    return "UNKNOWN";
}

} // namespace agoms
