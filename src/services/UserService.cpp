#include "services/UserService.h"
#include "models/AirportAdministrator.h"
#include "models/OperationsManager.h"
#include "models/GroundStaff.h"
#include "utils/Exceptions.h"

namespace agoms {
namespace services {

std::unique_ptr<User> UserService::login(const std::string& username, const std::string& password) {
    auto found = db_.findUserByUsername(username);
    if (!found.has_value()) {
        throw AuthenticationException("No such user: " + username);
    }
    std::unique_ptr<User> user = std::move(found.value());
    if (!user->isActive()) {
        throw AuthenticationException("Account is deactivated: " + username);
    }
    if (!user->verifyPassword(password)) {
        throw AuthenticationException("Incorrect password for user: " + username);
    }
    return user;
}

void UserService::createAdministrator(const std::string& userId, const std::string& username,
                                       const std::string& password, const std::string& fullName) {
    AirportAdministrator admin(userId, username, "", fullName);
    admin.setPassword(password);
    db_.saveUser(admin);
}

void UserService::createOperationsManager(const std::string& userId, const std::string& username,
                                           const std::string& password, const std::string& fullName) {
    OperationsManager mgr(userId, username, "", fullName);
    mgr.setPassword(password);
    db_.saveUser(mgr);
}

void UserService::createGroundStaff(const std::string& userId, const std::string& username,
                                     const std::string& password, const std::string& fullName,
                                     const std::string& linkedResourceId) {
    GroundStaff staff(userId, username, "", fullName, linkedResourceId);
    staff.setPassword(password);
    db_.saveUser(staff);
}

std::vector<std::unique_ptr<User>> UserService::listUsers() {
    return db_.listUsers();
}

} // namespace services
} // namespace agoms
