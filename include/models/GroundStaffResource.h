#ifndef AGOMS_GROUND_STAFF_RESOURCE_H
#define AGOMS_GROUND_STAFF_RESOURCE_H

#include "models/Resource.h"

namespace agoms {

// Allocatable ground-crew resource (e.g. "GS01"). Represents a crew member
// or crew slot that can be assigned to a flight/ground task. This is
// intentionally kept separate from the GroundStaff *user account* class
// (models/GroundStaff.h) — a login identity is not the same concept as a
// schedulable resource, even though in practice one often maps to the
// other via GroundStaff::getLinkedResourceId().
class GroundStaffResource : public Resource {
public:
    GroundStaffResource(std::string resourceId, std::string specialization)
        : Resource(resourceId, ResourceType::GROUND_STAFF, resourceId),
          specialization_(std::move(specialization)) {}

    const std::string& getSpecialization() const { return specialization_; }

    std::string describe() const override {
        return "Ground Staff " + resourceId_ + " (" + specialization_ + ") - " + toString(status_);
    }

private:
    std::string specialization_;
};

} // namespace agoms

#endif // AGOMS_GROUND_STAFF_RESOURCE_H
