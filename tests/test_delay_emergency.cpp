#include "TestFramework.h"
#include "repositories/AirportDatabase.h"
#include "services/FlightService.h"
#include "services/AllocationService.h"
#include "services/DelayService.h"
#include "services/EmergencyService.h"
#include "utils/Exceptions.h"

using namespace agoms;
using namespace agoms::services;

namespace {
struct DelayFixture {
    repo::AirportDatabase db{":memory:"};
    FlightService flights{db};
    AllocationService allocations{db};
    DelayService delays{db, flights, allocations};

    DelayFixture() {
        db.initializeSchema();
        flights.createFlight("FD01", "AI700", "DEL", "BOM", "A320",
                              util::DateTime::fromString("2026-09-02 09:00"),
                              util::DateTime::fromString("2026-09-02 10:30"));
        flights.transitionFlightStatus("FD01", FlightStatus::APPROACHING);
        flights.transitionFlightStatus("FD01", FlightStatus::ARRIVED);
        flights.transitionFlightStatus("FD01", FlightStatus::GROUND_OPERATIONS);
    }
};
}

// Test 9: Delay handling
TEST(Delay_ApplyDelayUpdatesStatusAndSchedule) {
    DelayFixture fx;
    DelayOutcome outcome = fx.delays.applyDelay("FD01", DelayReason::WEATHER, 45, "Storm on approach");

    ASSERT_EQ(std::string("FD01"), outcome.delay.getFlightId());
    ASSERT_EQ(45, outcome.delay.getDelayMinutes());
    ASSERT_TRUE(fx.flights.getFlight("FD01").getStatus() == FlightStatus::DELAYED);

    auto schedule = fx.flights.getSchedule("FD01");
    ASSERT_TRUE(schedule.has_value());
    // Estimated arrival should be 45 minutes after the original scheduled arrival.
    long diff = schedule->getEstimatedArrival().minutesSince(schedule->getScheduledArrival());
    ASSERT_EQ(45L, diff);
}

TEST(Delay_NonPositiveDurationRejected) {
    DelayFixture fx;
    ASSERT_THROWS(fx.delays.applyDelay("FD01", DelayReason::OTHER, 0, ""), ValidationException);
}

TEST(Delay_RecordedInHistory) {
    DelayFixture fx;
    fx.delays.applyDelay("FD01", DelayReason::ATC, 20, "ATC hold");
    auto history = fx.delays.listDelaysForFlight("FD01");
    ASSERT_EQ(static_cast<size_t>(1), history.size());
    ASSERT_TRUE(history[0].getReason() == DelayReason::ATC);
}

// Test 10: Emergency landing
namespace {
struct EmergencyFixture {
    repo::AirportDatabase db{":memory:"};
    FlightService flights{db};
    EmergencyService emergencies{db, flights};

    EmergencyFixture() {
        db.initializeSchema();
        flights.createFlight("FE01", "AI800", "DEL", "BOM", "A320",
                              util::DateTime::fromString("2026-09-03 09:00"),
                              util::DateTime::fromString("2026-09-03 10:30"));
        flights.transitionFlightStatus("FE01", FlightStatus::APPROACHING);
    }
};
}

TEST(Emergency_DeclareTransitionsFlightAndRecordsDetails) {
    EmergencyFixture fx;
    EmergencyLanding emergency = fx.emergencies.declareEmergency("FE01", "Engine failure");

    ASSERT_TRUE(fx.flights.getFlight("FE01").getStatus() == FlightStatus::EMERGENCY_LANDING);
    ASSERT_EQ(std::string("FE01"), emergency.getFlightId());
    ASSERT_TRUE(emergency.getStatus() == EmergencyStatus::DECLARED);
}

TEST(Emergency_FullWorkflowReachesGroundHandling) {
    EmergencyFixture fx;
    EmergencyLanding emergency = fx.emergencies.declareEmergency("FE01", "Hydraulic failure");

    EmergencyLanding landed = fx.emergencies.recordLanding(emergency.getEmergencyId());
    ASSERT_TRUE(landed.getStatus() == EmergencyStatus::LANDED);

    fx.emergencies.beginGroundHandling("FE01", emergency.getEmergencyId());
    ASSERT_TRUE(fx.flights.getFlight("FE01").getStatus() == FlightStatus::GROUND_OPERATIONS);

    fx.emergencies.resolveEmergency(emergency.getEmergencyId());
    EmergencyLanding resolved = fx.emergencies.getEmergency(emergency.getEmergencyId());
    ASSERT_TRUE(resolved.getStatus() == EmergencyStatus::RESOLVED);
}

TEST(Emergency_CannotDeclareFromScheduledState) {
    repo::AirportDatabase db(":memory:");
    db.initializeSchema();
    FlightService flights(db);
    EmergencyService emergencies(db, flights);
    flights.createFlight("FE02", "AI801", "DEL", "BOM", "A320",
                          util::DateTime::fromString("2026-09-03 09:00"),
                          util::DateTime::fromString("2026-09-03 10:30"));
    // Flight is still SCHEDULED (never transitioned to APPROACHING).
    ASSERT_THROWS(emergencies.declareEmergency("FE02", "Bird strike"), InvalidStateTransitionException);
}
