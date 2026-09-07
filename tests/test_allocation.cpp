#include "TestFramework.h"
#include "repositories/AirportDatabase.h"
#include "services/FlightService.h"
#include "services/ResourceService.h"
#include "services/AllocationService.h"
#include "utils/Exceptions.h"

using namespace agoms;
using namespace agoms::services;

namespace {

struct Fixture {
    repo::AirportDatabase db{":memory:"};
    FlightService flights{db};
    ResourceService resources{db};
    AllocationService allocations{db};

    Fixture() {
        db.initializeSchema();
        flights.createFlight("FA01", "AI900", "DEL", "BOM", "A320",
                              util::DateTime::fromString("2026-09-01 09:00"),
                              util::DateTime::fromString("2026-09-01 10:30"));
        flights.createFlight("FA02", "AI901", "BOM", "BLR", "A320",
                              util::DateTime::fromString("2026-09-01 09:15"),
                              util::DateTime::fromString("2026-09-01 10:45"));
        resources.addGate("GA01", "A");
        resources.addVehicle("VA01", "Baggage Tractor");
        resources.addGroundStaffResource("SA01", "Baggage Handling");
        resources.addGroundStaffResource("SA02", "Ramp Agent");
    }

    AllocationRequest requestFor(const std::string& flightId, std::string start, std::string end,
                                  std::vector<std::string> staff = {"SA01", "SA02"}) {
        AllocationRequest r;
        r.flightId = flightId;
        r.gateId = "GA01";
        r.vehicleId = "VA01";
        r.groundStaffIds = std::move(staff);
        r.windowStart = util::DateTime::fromString(start);
        r.windowEnd = util::DateTime::fromString(end);
        return r;
    }
};

} // namespace

// Test 3: Successful resource allocation
TEST(Allocation_SuccessfulAllocationOccupiesResourcesAndPersistsRecord) {
    Fixture fx;
    AllocationResult result = fx.allocations.allocate(fx.requestFor("FA01", "2026-09-01 09:00", "2026-09-01 10:30"));

    ASSERT_TRUE(result.isSuccess());
    ASSERT_FALSE(result.getAllocationId().empty());

    ASSERT_TRUE(fx.resources.getGate("GA01").getStatus() == ResourceStatus::OCCUPIED);
    ASSERT_TRUE(fx.resources.getVehicle("VA01").getStatus() == ResourceStatus::OCCUPIED);
    ASSERT_TRUE(fx.resources.getGroundStaffResource("SA01").getStatus() == ResourceStatus::OCCUPIED);
    ASSERT_TRUE(fx.resources.getGroundStaffResource("SA02").getStatus() == ResourceStatus::OCCUPIED);

    auto allocs = fx.allocations.listAllocationsForFlight("FA01");
    ASSERT_EQ(static_cast<size_t>(1), allocs.size());
}

// Test 4: Gate conflict
TEST(Allocation_OverlappingGateRequestFails) {
    Fixture fx;
    AllocationResult first = fx.allocations.allocate(fx.requestFor("FA01", "2026-09-01 09:00", "2026-09-01 10:30"));
    ASSERT_TRUE(first.isSuccess());

    // Different flight, same gate, overlapping window -> must fail.
    AllocationRequest second = fx.requestFor("FA02", "2026-09-01 09:15", "2026-09-01 10:00");
    AllocationResult result = fx.allocations.allocate(second);

    ASSERT_FALSE(result.isSuccess());
    ASSERT_TRUE(result.getConflict().getConflictType() == ConflictType::GATE_UNAVAILABLE);
}

// Test 5: Vehicle conflict
TEST(Allocation_OverlappingVehicleRequestFails) {
    Fixture fx;
    fx.resources.addGate("GA02", "B"); // different gate so only the vehicle collides
    AllocationRequest first = fx.requestFor("FA01", "2026-09-01 09:00", "2026-09-01 10:30");
    ASSERT_TRUE(fx.allocations.allocate(first).isSuccess());

    AllocationRequest second = fx.requestFor("FA02", "2026-09-01 09:15", "2026-09-01 10:00");
    second.gateId = "GA02"; // avoid the gate conflict so we isolate the vehicle conflict
    AllocationResult result = fx.allocations.allocate(second);

    ASSERT_FALSE(result.isSuccess());
    ASSERT_TRUE(result.getConflict().getConflictType() == ConflictType::VEHICLE_UNAVAILABLE);
}

// Test 6: Ground staff conflict
TEST(Allocation_OverlappingGroundStaffRequestFails) {
    Fixture fx;
    fx.resources.addGate("GA03", "B");
    fx.resources.addVehicle("VA02", "Fuel Truck");

    AllocationRequest first = fx.requestFor("FA01", "2026-09-01 09:00", "2026-09-01 10:30", {"SA01"});
    ASSERT_TRUE(fx.allocations.allocate(first).isSuccess());

    AllocationRequest second = fx.requestFor("FA02", "2026-09-01 09:15", "2026-09-01 10:00", {"SA01"});
    second.gateId = "GA03";
    second.vehicleId = "VA02";
    AllocationResult result = fx.allocations.allocate(second);

    ASSERT_FALSE(result.isSuccess());
    ASSERT_TRUE(result.getConflict().getConflictType() == ConflictType::GROUND_STAFF_UNAVAILABLE);
}

TEST(Allocation_NonOverlappingWindowsOnSameGateSucceed) {
    Fixture fx;
    AllocationRequest first = fx.requestFor("FA01", "2026-09-01 09:00", "2026-09-01 10:00", {"SA01"});
    ASSERT_TRUE(fx.allocations.allocate(first).isSuccess());

    AllocationRequest second = fx.requestFor("FA02", "2026-09-01 10:00", "2026-09-01 11:00", {"SA02"});
    AllocationResult result = fx.allocations.allocate(second);
    ASSERT_TRUE(result.isSuccess());
}

// Test 7: Resource release
TEST(Allocation_ReleaseReturnsResourcesToAvailable) {
    Fixture fx;
    AllocationResult result = fx.allocations.allocate(fx.requestFor("FA01", "2026-09-01 09:00", "2026-09-01 10:30"));
    ASSERT_TRUE(result.isSuccess());
    ASSERT_TRUE(fx.resources.getGate("GA01").getStatus() == ResourceStatus::OCCUPIED);

    fx.allocations.release(result.getAllocationId());

    ASSERT_TRUE(fx.resources.getGate("GA01").getStatus() == ResourceStatus::AVAILABLE);
    ASSERT_TRUE(fx.resources.getVehicle("VA01").getStatus() == ResourceStatus::AVAILABLE);
    ASSERT_TRUE(fx.resources.getGroundStaffResource("SA01").getStatus() == ResourceStatus::AVAILABLE);
    ASSERT_TRUE(fx.resources.getGroundStaffResource("SA02").getStatus() == ResourceStatus::AVAILABLE);

    // After release, a new flight should be able to allocate the same gate for the same window.
    AllocationRequest again = fx.requestFor("FA02", "2026-09-01 09:00", "2026-09-01 10:30");
    ASSERT_TRUE(fx.allocations.allocate(again).isSuccess());
}

TEST(Allocation_ReleasingAlreadyReleasedAllocationThrows) {
    Fixture fx;
    AllocationResult result = fx.allocations.allocate(fx.requestFor("FA01", "2026-09-01 09:00", "2026-09-01 10:30"));
    fx.allocations.release(result.getAllocationId());
    ASSERT_THROWS(fx.allocations.release(result.getAllocationId()), ValidationException);
}

TEST(Allocation_UnknownFlightFails) {
    Fixture fx;
    AllocationRequest req = fx.requestFor("NOT_A_FLIGHT", "2026-09-01 09:00", "2026-09-01 10:30");
    AllocationResult result = fx.allocations.allocate(req);
    ASSERT_FALSE(result.isSuccess());
}
