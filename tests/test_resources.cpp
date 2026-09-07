#include "TestFramework.h"
#include "repositories/AirportDatabase.h"
#include "services/ResourceService.h"
#include "utils/Exceptions.h"

using namespace agoms;
using namespace agoms::services;

namespace {
repo::AirportDatabase freshDb() {
    repo::AirportDatabase db(":memory:");
    db.initializeSchema();
    return db;
}
}

// Test 2: Resource creation
TEST(ResourceCreation_GateVehicleStaffAllPersist) {
    repo::AirportDatabase db = freshDb();
    ResourceService resources(db);

    resources.addGate("GX01", "A");
    resources.addVehicle("VX01", "Fuel Truck");
    resources.addGroundStaffResource("SX01", "Ramp Agent");

    Gate gate = resources.getGate("GX01");
    Vehicle vehicle = resources.getVehicle("VX01");
    GroundStaffResource staff = resources.getGroundStaffResource("SX01");

    ASSERT_TRUE(gate.getStatus() == ResourceStatus::AVAILABLE);
    ASSERT_TRUE(vehicle.getStatus() == ResourceStatus::AVAILABLE);
    ASSERT_TRUE(staff.getStatus() == ResourceStatus::AVAILABLE);

    ASSERT_EQ(static_cast<size_t>(1), resources.listGates().size());
    ASSERT_EQ(static_cast<size_t>(1), resources.listVehicles().size());
    ASSERT_EQ(static_cast<size_t>(1), resources.listGroundStaff().size());
}

TEST(ResourceCreation_DuplicateGateRejected) {
    repo::AirportDatabase db = freshDb();
    ResourceService resources(db);
    resources.addGate("GX02", "B");
    ASSERT_THROWS(resources.addGate("GX02", "B"), ValidationException);
}

TEST(ResourceService_SetStatusToMaintenance) {
    repo::AirportDatabase db = freshDb();
    ResourceService resources(db);
    resources.addGate("GX03", "A");
    resources.setGateStatus("GX03", ResourceStatus::MAINTENANCE);
    ASSERT_TRUE(resources.getGate("GX03").getStatus() == ResourceStatus::MAINTENANCE);
}

TEST(ResourceService_UnknownGateThrows) {
    repo::AirportDatabase db = freshDb();
    ResourceService resources(db);
    ASSERT_THROWS(resources.getGate("NOPE"), NotFoundException);
}
