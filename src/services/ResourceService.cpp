#include "services/ResourceService.h"
#include "utils/Exceptions.h"

namespace agoms {
namespace services {

void ResourceService::addGate(const std::string& id, const std::string& terminal) {
    if (db_.findGate(id).has_value()) throw ValidationException("Gate already exists: " + id);
    db_.saveGate(Gate(id, terminal));
}

void ResourceService::addVehicle(const std::string& id, const std::string& vehicleClass) {
    if (db_.findVehicle(id).has_value()) throw ValidationException("Vehicle already exists: " + id);
    db_.saveVehicle(Vehicle(id, vehicleClass));
}

void ResourceService::addGroundStaffResource(const std::string& id, const std::string& specialization) {
    if (db_.findGroundStaffResource(id).has_value())
        throw ValidationException("Ground staff resource already exists: " + id);
    db_.saveGroundStaffResource(GroundStaffResource(id, specialization));
}

Gate ResourceService::getGate(const std::string& id) const {
    auto g = db_.findGate(id);
    if (!g.has_value()) throw NotFoundException("Gate not found: " + id);
    return g.value();
}

Vehicle ResourceService::getVehicle(const std::string& id) const {
    auto v = db_.findVehicle(id);
    if (!v.has_value()) throw NotFoundException("Vehicle not found: " + id);
    return v.value();
}

GroundStaffResource ResourceService::getGroundStaffResource(const std::string& id) const {
    auto s = db_.findGroundStaffResource(id);
    if (!s.has_value()) throw NotFoundException("Ground staff resource not found: " + id);
    return s.value();
}

void ResourceService::setGateStatus(const std::string& id, ResourceStatus status) {
    getGate(id); // validates existence
    db_.updateGateStatus(id, status);
}

void ResourceService::setVehicleStatus(const std::string& id, ResourceStatus status) {
    getVehicle(id);
    db_.updateVehicleStatus(id, status);
}

void ResourceService::setGroundStaffStatus(const std::string& id, ResourceStatus status) {
    getGroundStaffResource(id);
    db_.updateGroundStaffResourceStatus(id, status);
}

} // namespace services
} // namespace agoms
