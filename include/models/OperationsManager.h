#ifndef AGOMS_OPERATIONS_MANAGER_H
#define AGOMS_OPERATIONS_MANAGER_H

#include "models/User.h"

namespace agoms {

// Operations Manager: performs day-to-day operational activities such as
// resource allocation, conflict resolution, delay handling, emergency
// landing response and schedule generation.
class OperationsManager : public User {
public:
    OperationsManager(std::string userId, std::string username,
                       std::string passwordHash, std::string fullName)
        : User(std::move(userId), std::move(username), std::move(passwordHash),
               std::move(fullName), UserRole::OPERATIONS_MANAGER) {}

    std::string describeRole() const override { return "Operations Manager"; }
};

} // namespace agoms

#endif // AGOMS_OPERATIONS_MANAGER_H
