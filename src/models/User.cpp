#include "models/User.h"
#include <functional>
#include <sstream>

namespace agoms {

namespace {
// NOTE: This is a simple, deterministic hash used ONLY for academic/demo
// purposes so that plaintext passwords are never stored verbatim in the
// database. It is NOT cryptographically secure and must not be used in a
// production system (a real system would use bcrypt/argon2 + per-user salt).
std::string simpleHash(const std::string& input) {
    std::hash<std::string> hasher;
    std::ostringstream oss;
    oss << std::hex << hasher(input);
    return oss.str();
}
}

User::User(std::string userId, std::string username, std::string passwordHash,
           std::string fullName, UserRole role)
    : userId_(std::move(userId)),
      username_(std::move(username)),
      passwordHash_(std::move(passwordHash)),
      fullName_(std::move(fullName)),
      role_(role) {}

bool User::verifyPassword(const std::string& plaintextPassword) const {
    return passwordHash_ == simpleHash(plaintextPassword);
}

void User::setPassword(const std::string& plaintextPassword) {
    passwordHash_ = simpleHash(plaintextPassword);
}

} // namespace agoms
