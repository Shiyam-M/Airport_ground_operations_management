#include "TestFramework.h"
#include "repositories/AirportDatabase.h"
#include "services/FlightService.h"
#include "services/UserService.h"
#include "utils/Exceptions.h"
#include <cstdio>

using namespace agoms;
using namespace agoms::services;

// Test 11: Database persistence
// Verifies that data survives closing and re-opening the SAME on-disk
// SQLite file (as opposed to an in-memory ":memory:" database, which is
// used by the other test files for fast, isolated tests).
TEST(Database_DataSurvivesReopen) {
    const std::string path = "test_persistence_tmp.db";
    std::remove(path.c_str());

    {
        repo::AirportDatabase db(path);
        db.initializeSchema();
        FlightService flights(db);
        flights.createFlight("FP01", "AI999", "DEL", "GOI", "A321",
                              util::DateTime::fromString("2026-09-05 09:00"),
                              util::DateTime::fromString("2026-09-05 10:30"));
        flights.transitionFlightStatus("FP01", FlightStatus::APPROACHING);
    } // db goes out of scope here -> connection closed, file flushed to disk

    {
        repo::AirportDatabase db(path);
        db.initializeSchema(); // idempotent; tables already exist
        FlightService flights(db);
        Flight flight = flights.getFlight("FP01");
        ASSERT_EQ(std::string("FP01"), flight.getFlightId());
        ASSERT_TRUE(flight.getStatus() == FlightStatus::APPROACHING);
    }

    std::remove(path.c_str());
}

TEST(Database_SeedDataIsIdempotent) {
    const std::string path = "test_seed_tmp.db";
    std::remove(path.c_str());

    {
        repo::AirportDatabase db(path);
        db.initializeSchema();
        db.seedSampleDataIfEmpty();
    }
    {
        // Re-opening and seeding again must not throw or duplicate rows,
        // since seedSampleDataIfEmpty() checks the flights table first.
        repo::AirportDatabase db(path);
        db.initializeSchema();
        db.seedSampleDataIfEmpty();
        auto flights = db.listFlights();
        ASSERT_EQ(static_cast<size_t>(3), flights.size()); // FL001, FL002, FL003 only, not duplicated
    }

    std::remove(path.c_str());
}

TEST(Database_UserAuthenticationRoundTrips) {
    repo::AirportDatabase db(":memory:");
    db.initializeSchema();
    UserService users(db);
    users.createOperationsManager("UX01", "opstest", "secret123", "Test Manager");

    auto user = users.login("opstest", "secret123");
    ASSERT_EQ(std::string("Test Manager"), user->getFullName());
    ASSERT_TRUE(user->getRole() == UserRole::OPERATIONS_MANAGER);

    ASSERT_THROWS(users.login("opstest", "wrongpassword"), AuthenticationException);
    ASSERT_THROWS(users.login("nosuchuser", "secret123"), AuthenticationException);
}
