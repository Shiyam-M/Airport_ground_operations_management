// main.cpp
// Entry point for the Airport Ground Operations Management System (AGOMS).
// Responsible only for wiring the persistence layer to the presentation
// layer and starting the console loop -- all business logic lives in the
// services/ layer, and all persistence lives in repositories/AirportDatabase.
#include <iostream>
#include "repositories/AirportDatabase.h"
#include "controllers/ConsoleUI.h"

int main(int argc, char** argv) {
    std::string dbPath = "agoms.db";
    if (argc > 1) {
        dbPath = argv[1];
    }

    try {
        agoms::repo::AirportDatabase db(dbPath);
        db.initializeSchema();
        db.seedSampleDataIfEmpty();

        agoms::controllers::ConsoleUI ui(db);
        ui.run();
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
