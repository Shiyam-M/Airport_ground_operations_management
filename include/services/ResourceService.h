#ifndef AGOMS_RESOURCE_SERVICE_H
#define AGOMS_RESOURCE_SERVICE_H

#include <string>
#include <vector>
#include "models/Gate.h"
#include "models/Vehicle.h"
#include "models/GroundStaffResource.h"
#include "repositories/AirportDatabase.h"

namespace agoms {
namespace services {

// Manages the resource master data (Gates, Vehicles, Ground Staff
// resources) and their AVAILABLE/OCCUPIED/MAINTENANCE lifecycle.
// AllocationService is the only other component allowed to flip a
// resource's status as a side effect of an allocation/release; direct
// admin-driven status changes (e.g. putting a gate into MAINTENANCE) also
// go through here.
class ResourceService {
public:
    explicit ResourceService(repo::AirportDatabase& db) : db_(db) {}

    void addGate(const std::string& id, const std::string& terminal);
    void addVehicle(const std::string& id, const std::string& vehicleClass);
    void addGroundStaffResource(const std::string& id, const std::string& specialization);

    std::vector<Gate> listGates() const { return db_.listGates(); }
    std::vector<Vehicle> listVehicles() const { return db_.listVehicles(); }
    std::vector<GroundStaffResource> listGroundStaff() const { return db_.listGroundStaffResources(); }

    Gate getGate(const std::string& id) const;               // throws NotFoundException
    Vehicle getVehicle(const std::string& id) const;          // throws NotFoundException
    GroundStaffResource getGroundStaffResource(const std::string& id) const; // throws NotFoundException

    void setGateStatus(const std::string& id, ResourceStatus status);
    void setVehicleStatus(const std::string& id, ResourceStatus status);
    void setGroundStaffStatus(const std::string& id, ResourceStatus status);

private:
    repo::AirportDatabase& db_;
};

} // namespace services
} // namespace agoms

#endif // AGOMS_RESOURCE_SERVICE_H
