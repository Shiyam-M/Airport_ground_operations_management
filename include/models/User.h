#ifndef AGOMS_USER_H
#define AGOMS_USER_H

#include <string>
#include "models/Enums.h"

namespace agoms {

// Base class for all system actors (encapsulates shared account data).
// Concrete subclasses (AirportAdministrator, OperationsManager, GroundStaff)
// add role-specific behaviour on top of this common identity/auth surface.
class User {
public:
    User(std::string userId, std::string username, std::string passwordHash,
         std::string fullName, UserRole role);
    virtual ~User() = default;

    const std::string& getUserId() const { return userId_; }
    const std::string& getUsername() const { return username_; }
    const std::string& getFullName() const { return fullName_; }
    UserRole getRole() const { return role_; }
    bool isActive() const { return active_; }

    void setFullName(const std::string& name) { fullName_ = name; }
    void setActive(bool active) { active_ = active; }

    // Compares a plaintext password against the stored hash.
    bool verifyPassword(const std::string& plaintextPassword) const;
    void setPassword(const std::string& plaintextPassword);

    const std::string& getPasswordHash() const { return passwordHash_; }

    // Human-readable role description; overridden by subclasses for UI display.
    virtual std::string describeRole() const = 0;

protected:
    std::string userId_;
    std::string username_;
    std::string passwordHash_;
    std::string fullName_;
    UserRole role_;
    bool active_ = true;
};

} // namespace agoms

#endif // AGOMS_USER_H
