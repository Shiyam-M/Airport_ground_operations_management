#include "TestFramework.h"
#include "repositories/AirportDatabase.h"
#include "services/FlightService.h"
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

// Test 1: Flight creation
TEST(FlightCreation_PersistsAndIsRetrievable) {
    repo::AirportDatabase db = freshDb();
    FlightService flightService(db);

    flightService.createFlight("FLX01", "TT100", "DEL", "BOM", "A320",
                                util::DateTime::fromString("2026-09-01 09:00"),
                                util::DateTime::fromString("2026-09-01 10:30"));

    Flight flight = flightService.getFlight("FLX01");
    ASSERT_EQ(std::string("FLX01"), flight.getFlightId());
    ASSERT_EQ(std::string("TT100"), flight.getFlightNumber());
    ASSERT_TRUE(flight.getStatus() == FlightStatus::SCHEDULED);

    auto schedule = flightService.getSchedule("FLX01");
    ASSERT_TRUE(schedule.has_value());
}

TEST(FlightCreation_DuplicateIdRejected) {
    repo::AirportDatabase db = freshDb();
    FlightService flightService(db);
    flightService.createFlight("FLX02", "TT200", "DEL", "BOM", "A320",
                                util::DateTime::fromString("2026-09-01 09:00"),
                                util::DateTime::fromString("2026-09-01 10:30"));
    ASSERT_THROWS(
        flightService.createFlight("FLX02", "TT201", "DEL", "BOM", "A320",
                                    util::DateTime::fromString("2026-09-01 09:00"),
                                    util::DateTime::fromString("2026-09-01 10:30")),
        ValidationException);
}

// Test 8: Flight state transition
TEST(FlightStateTransition_ValidChainSucceeds) {
    repo::AirportDatabase db = freshDb();
    FlightService flightService(db);
    flightService.createFlight("FLX03", "TT300", "DEL", "BOM", "A320",
                                util::DateTime::fromString("2026-09-01 09:00"),
                                util::DateTime::fromString("2026-09-01 10:30"));

    flightService.transitionFlightStatus("FLX03", FlightStatus::APPROACHING);
    ASSERT_TRUE(flightService.getFlight("FLX03").getStatus() == FlightStatus::APPROACHING);

    flightService.transitionFlightStatus("FLX03", FlightStatus::ARRIVED);
    ASSERT_TRUE(flightService.getFlight("FLX03").getStatus() == FlightStatus::ARRIVED);

    flightService.transitionFlightStatus("FLX03", FlightStatus::GROUND_OPERATIONS);
    flightService.transitionFlightStatus("FLX03", FlightStatus::BOARDING);
    flightService.transitionFlightStatus("FLX03", FlightStatus::READY_FOR_DEPARTURE);
    flightService.transitionFlightStatus("FLX03", FlightStatus::DEPARTED);
    flightService.transitionFlightStatus("FLX03", FlightStatus::COMPLETED);

    ASSERT_TRUE(flightService.getFlight("FLX03").getStatus() == FlightStatus::COMPLETED);
}

TEST(FlightStateTransition_IllegalTransitionRejected) {
    repo::AirportDatabase db = freshDb();
    FlightService flightService(db);
    flightService.createFlight("FLX04", "TT400", "DEL", "BOM", "A320",
                                util::DateTime::fromString("2026-09-01 09:00"),
                                util::DateTime::fromString("2026-09-01 10:30"));

    // SCHEDULED -> DEPARTED is not a legal direct transition.
    ASSERT_THROWS(flightService.transitionFlightStatus("FLX04", FlightStatus::DEPARTED),
                  InvalidStateTransitionException);
}

TEST(FlightStateTransition_EmergencyPathAllowed) {
    repo::AirportDatabase db = freshDb();
    FlightService flightService(db);
    flightService.createFlight("FLX05", "TT500", "DEL", "BOM", "A320",
                                util::DateTime::fromString("2026-09-01 09:00"),
                                util::DateTime::fromString("2026-09-01 10:30"));

    flightService.transitionFlightStatus("FLX05", FlightStatus::APPROACHING);
    flightService.transitionFlightStatus("FLX05", FlightStatus::EMERGENCY_LANDING);
    ASSERT_TRUE(flightService.getFlight("FLX05").getStatus() == FlightStatus::EMERGENCY_LANDING);
    flightService.transitionFlightStatus("FLX05", FlightStatus::GROUND_OPERATIONS);
    ASSERT_TRUE(flightService.getFlight("FLX05").getStatus() == FlightStatus::GROUND_OPERATIONS);
}

TEST(FlightService_GetFlightThrowsWhenNotFound) {
    repo::AirportDatabase db = freshDb();
    FlightService flightService(db);
    ASSERT_THROWS(flightService.getFlight("DOES_NOT_EXIST"), NotFoundException);
}
