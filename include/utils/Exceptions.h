#ifndef AGOMS_EXCEPTIONS_H
#define AGOMS_EXCEPTIONS_H

#include <stdexcept>
#include <string>

namespace agoms {

// Base class for all application-defined (recoverable) errors. Caught at the
// controller/UI boundary so the console app never crashes on bad input or
// expected business-rule violations.
class AgomsException : public std::runtime_error {
public:
    explicit AgomsException(const std::string& message) : std::runtime_error(message) {}
};

class NotFoundException : public AgomsException {
public:
    explicit NotFoundException(const std::string& message) : AgomsException(message) {}
};

class ValidationException : public AgomsException {
public:
    explicit ValidationException(const std::string& message) : AgomsException(message) {}
};

class ResourceUnavailableException : public AgomsException {
public:
    explicit ResourceUnavailableException(const std::string& message) : AgomsException(message) {}
};

class AllocationConflictException : public AgomsException {
public:
    explicit AllocationConflictException(const std::string& message) : AgomsException(message) {}
};

class InvalidStateTransitionException : public AgomsException {
public:
    explicit InvalidStateTransitionException(const std::string& message) : AgomsException(message) {}
};

class DatabaseException : public AgomsException {
public:
    explicit DatabaseException(const std::string& message) : AgomsException(message) {}
};

class AuthenticationException : public AgomsException {
public:
    explicit AuthenticationException(const std::string& message) : AgomsException(message) {}
};

} // namespace agoms

#endif // AGOMS_EXCEPTIONS_H
