-- AGOMS sample seed data
-- This mirrors the data programmatically inserted by
-- AirportDatabase::seedSampleData() when the app is run for the first time
-- against a fresh database file. Provided here for reference / manual use:
--   sqlite3 agoms.db < database/schema.sql
--   sqlite3 agoms.db < database/seed.sql

-- Users (password for all sample accounts is shown in README.md)
INSERT OR IGNORE INTO users (user_id, username, password_hash, full_name, role, linked_resource_id, active)
VALUES
 ('U0001','admin',   '', 'Alice Admin',      'ADMINISTRATOR',      NULL,    1),
 ('U0002','opsmgr',  '', 'Oscar Operations', 'OPERATIONS_MANAGER', NULL,    1),
 ('U0003','staff1',  '', 'Gina Ground',      'GROUND_STAFF',       'GS01',  1),
 ('U0004','staff2',  '', 'Sam Ground',       'GROUND_STAFF',       'GS02',  1);
-- NOTE: password_hash values above are placeholders; the running
-- application seeds real hashed passwords via User::setPassword() so that
-- the simple-hash algorithm implementation stays in one place (models/User.cpp).

-- Gates
INSERT OR IGNORE INTO gates (resource_id, terminal, status) VALUES
 ('G01','A','AVAILABLE'),
 ('G02','A','AVAILABLE'),
 ('G03','B','AVAILABLE');

-- Vehicles
INSERT OR IGNORE INTO vehicles (resource_id, vehicle_class, status) VALUES
 ('V01','Baggage Tractor','AVAILABLE'),
 ('V02','Fuel Truck','AVAILABLE');

-- Ground staff resources
INSERT OR IGNORE INTO ground_staff_resources (resource_id, specialization, status) VALUES
 ('GS01','Baggage Handling','AVAILABLE'),
 ('GS02','Ramp Agent','AVAILABLE'),
 ('GS03','Cleaning Crew','AVAILABLE'),
 ('GS04','Catering Crew','AVAILABLE');

-- Flights
INSERT OR IGNORE INTO flights (flight_id, flight_number, origin, destination, aircraft_type, status) VALUES
 ('FL001','AI101','DEL','BOM','A320','SCHEDULED'),
 ('FL002','AI202','BOM','BLR','B737','SCHEDULED'),
 ('FL003','AI303','BLR','MAA','A321','SCHEDULED');

-- Flight schedules
INSERT OR IGNORE INTO flight_schedules (schedule_id, flight_id, scheduled_arrival, scheduled_departure, estimated_arrival, estimated_departure)
VALUES
 ('SC0001','FL001','2026-08-26 09:00','2026-08-26 10:30','2026-08-26 09:00','2026-08-26 10:30'),
 ('SC0002','FL002','2026-08-26 11:00','2026-08-26 12:30','2026-08-26 11:00','2026-08-26 12:30'),
 ('SC0003','FL003','2026-08-26 13:00','2026-08-26 14:30','2026-08-26 13:00','2026-08-26 14:30');
