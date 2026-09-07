#ifndef AGOMS_VEHICLE_H
#define AGOMS_VEHICLE_H

#include "models/Resource.h"

namespace agoms {

// Ground service vehicle (fuel truck, baggage cart, catering truck, pushback
// tug, etc.). vehicleClass_ is a free-text category used for display and
// reporting purposes only.
class Vehicle : public Resource {
public:
    Vehicle(std::string resourceId, std::string vehicleClass)
        : Resource(resourceId, ResourceType::VEHICLE, resourceId),
          vehicleClass_(std::move(vehicleClass)) {}

    const std::string& getVehicleClass() const { return vehicleClass_; }

    std::string describe() const override {
        return "Vehicle " + resourceId_ + " (" + vehicleClass_ + ") - " + toString(status_);
    }

private:
    std::string vehicleClass_;
};

} // namespace agoms

#endif // AGOMS_VEHICLE_H
