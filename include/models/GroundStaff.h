#ifndef AGOMS_GROUND_STAFF_H
#define AGOMS_GROUND_STAFF_H

#include "models/User.h"

namespace agoms {

// Ground Staff (actor/user account). Distinct from GroundStaffResource
// (include/models/GroundStaffResource.h), which represents an allocatable
// crew resource that can be assigned to a flight/task. A GroundStaff user
// account is optionally linked to the GroundStaffResource ID that
// represents them on the operations floor, so they can log in and see
// "their" assigned tasks.
class GroundStaff : public User {
public:
    GroundStaff(std::string userId, std::string username, std::string passwordHash,
                std::string fullName, std::string linkedResourceId = "")
        : User(std::move(userId), std::move(username), std::move(passwordHash),
               std::move(fullName), UserRole::GROUND_STAFF),
          linkedResourceId_(std::move(linkedResourceId)) {}

    std::string describeRole() const override { return "Ground Staff"; }

    const std::string& getLinkedResourceId() const { return linkedResourceId_; }
    void setLinkedResourceId(const std::string& id) { linkedResourceId_ = id; }

private:
    std::string linkedResourceId_; // ID of the corresponding GroundStaffResource
};

} // namespace agoms

#endif // AGOMS_GROUND_STAFF_H
