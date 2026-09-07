#ifndef AGOMS_AIRPORT_ADMINISTRATOR_H
#define AGOMS_AIRPORT_ADMINISTRATOR_H

#include "models/User.h"

namespace agoms {

// Administrator: responsible for master-data management (flights, gates,
// vehicles, ground staff records). Business rules for what an administrator
// may do live in the service layer; this class only carries identity/role.
class AirportAdministrator : public User {
public:
    AirportAdministrator(std::string userId, std::string username,
                          std::string passwordHash, std::string fullName)
        : User(std::move(userId), std::move(username), std::move(passwordHash),
               std::move(fullName), UserRole::ADMINISTRATOR) {}

    std::string describeRole() const override { return "Airport Administrator"; }
};

} // namespace agoms

#endif // AGOMS_AIRPORT_ADMINISTRATOR_H
