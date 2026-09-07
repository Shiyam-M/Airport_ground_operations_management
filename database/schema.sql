-- AGOMS SQLite schema
-- This file documents the persistence model used by AirportDatabase.
-- The same DDL is executed programmatically by
-- AirportDatabase::initializeSchema() at application startup, so this file
-- is kept in sync with the C++ implementation as living documentation and
-- can also be run manually against an empty database file:
--   sqlite3 agoms.db < database/schema.sql

PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS users (
    user_id        TEXT PRIMARY KEY,
    username       TEXT NOT NULL UNIQUE,
    password_hash  TEXT NOT NULL,
    full_name      TEXT NOT NULL,
    role           TEXT NOT NULL,               -- ADMINISTRATOR | OPERATIONS_MANAGER | GROUND_STAFF
    linked_resource_id TEXT,                    -- only meaningful for GROUND_STAFF
    active         INTEGER NOT NULL DEFAULT 1
);

CREATE TABLE IF NOT EXISTS flights (
    flight_id      TEXT PRIMARY KEY,
    flight_number  TEXT NOT NULL,
    origin         TEXT NOT NULL,
    destination    TEXT NOT NULL,
    aircraft_type  TEXT NOT NULL,
    status         TEXT NOT NULL DEFAULT 'SCHEDULED'
);

CREATE TABLE IF NOT EXISTS flight_schedules (
    schedule_id         TEXT PRIMARY KEY,
    flight_id           TEXT NOT NULL REFERENCES flights(flight_id),
    scheduled_arrival    TEXT NOT NULL,          -- "YYYY-MM-DD HH:MM"
    scheduled_departure  TEXT NOT NULL,
    estimated_arrival    TEXT NOT NULL,
    estimated_departure  TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS gates (
    resource_id   TEXT PRIMARY KEY,
    terminal      TEXT NOT NULL,
    status        TEXT NOT NULL DEFAULT 'AVAILABLE'
);

CREATE TABLE IF NOT EXISTS vehicles (
    resource_id   TEXT PRIMARY KEY,
    vehicle_class TEXT NOT NULL,
    status        TEXT NOT NULL DEFAULT 'AVAILABLE'
);

CREATE TABLE IF NOT EXISTS ground_staff_resources (
    resource_id     TEXT PRIMARY KEY,
    specialization  TEXT NOT NULL,
    status          TEXT NOT NULL DEFAULT 'AVAILABLE'
);

CREATE TABLE IF NOT EXISTS allocations (
    allocation_id  TEXT PRIMARY KEY,
    flight_id      TEXT NOT NULL REFERENCES flights(flight_id),
    gate_id        TEXT NOT NULL REFERENCES gates(resource_id),
    vehicle_id     TEXT NOT NULL REFERENCES vehicles(resource_id),
    window_start   TEXT NOT NULL,
    window_end     TEXT NOT NULL,
    status         TEXT NOT NULL DEFAULT 'SUCCESS'   -- SUCCESS | RELEASED
);

-- Many-to-many: an allocation can reserve several ground staff resources.
CREATE TABLE IF NOT EXISTS allocation_staff (
    allocation_id  TEXT NOT NULL REFERENCES allocations(allocation_id),
    staff_id       TEXT NOT NULL REFERENCES ground_staff_resources(resource_id),
    PRIMARY KEY (allocation_id, staff_id)
);

CREATE TABLE IF NOT EXISTS ground_tasks (
    task_id        TEXT PRIMARY KEY,
    flight_id      TEXT NOT NULL REFERENCES flights(flight_id),
    task_type      TEXT NOT NULL,
    assigned_staff TEXT,
    status         TEXT NOT NULL DEFAULT 'PENDING',
    start_time     TEXT NOT NULL,
    end_time       TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS flight_delays (
    delay_id       TEXT PRIMARY KEY,
    flight_id      TEXT NOT NULL REFERENCES flights(flight_id),
    reason         TEXT NOT NULL,
    delay_minutes  INTEGER NOT NULL,
    notes          TEXT,
    recorded_at    TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS emergency_landings (
    emergency_id   TEXT PRIMARY KEY,
    flight_id      TEXT NOT NULL REFERENCES flights(flight_id),
    reason         TEXT NOT NULL,
    declared_at    TEXT NOT NULL,
    landing_time   TEXT,
    status         TEXT NOT NULL DEFAULT 'DECLARED'
);

CREATE TABLE IF NOT EXISTS weather_log (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    temperature_c   REAL NOT NULL,
    wind_speed_kmh  REAL NOT NULL,
    visibility_km   REAL NOT NULL,
    precipitation_mm REAL NOT NULL,
    condition       TEXT NOT NULL,
    observed_at     TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS atc_log (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    flight_id       TEXT NOT NULL,
    clearance       TEXT NOT NULL,
    departure_info  TEXT,
    arrival_info    TEXT,
    remarks         TEXT,
    logged_at       TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS reports (
    report_id     TEXT PRIMARY KEY,
    report_type   TEXT NOT NULL,
    title         TEXT NOT NULL,
    generated_at  TEXT NOT NULL,
    content       TEXT NOT NULL
);
