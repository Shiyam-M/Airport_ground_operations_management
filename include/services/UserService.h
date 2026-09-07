#ifndef AGOMS_USER_SERVICE_H
#define AGOMS_USER_SERVICE_H

#include <memory>
#include <string>
#include <vector>
#include "models/User.h"
#include "repositories/AirportDatabase.h"

namespace agoms {
namespace services {

// Handles authentication and (administrator-only) account management.
class UserService {
public:
    explicit UserService(repo::AirportDatabase& db) : db_(db) {}

    // Throws AuthenticationException on bad credentials or inactive account.
    std::unique_ptr<User> login(const std::string& username, const std::string& password);

    void createAdministrator(const std::string& userId, const std::string& username,
                              const std::string& password, const std::string& fullName);
    void createOperationsManager(const std::string& userId, const std::string& username,
                                  const std::string& password, const std::string& fullName);
    void createGroundStaff(const std::string& userId, const std::string& username,
                            const std::string& password, const std::string& fullName,
                            const std::string& linkedResourceId);

    std::vector<std::unique_ptr<User>> listUsers();

private:
    repo::AirportDatabase& db_;
};

} // namespace services
} // namespace agoms

#endif // AGOMS_USER_SERVICE_H
