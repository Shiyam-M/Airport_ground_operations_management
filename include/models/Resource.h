#ifndef AGOMS_RESOURCE_H
#define AGOMS_RESOURCE_H

#include <string>
#include "models/Enums.h"

namespace agoms {

// Abstract base for anything that can be allocated to a flight: gates,
// vehicles, and ground staff resources. Encapsulates the common
// id/status/type fields and status-transition guard logic.
class Resource {
public:
    Resource(std::string resourceId, ResourceType type, std::string label)
        : resourceId_(std::move(resourceId)), type_(type), label_(std::move(label)),
          status_(ResourceStatus::AVAILABLE) {}

    virtual ~Resource() = default;

    const std::string& getResourceId() const { return resourceId_; }
    ResourceType getType() const { return type_; }
    const std::string& getLabel() const { return label_; }

    ResourceStatus getStatus() const { return status_; }
    void setStatus(ResourceStatus status) { status_ = status; }

    bool isAvailable() const { return status_ == ResourceStatus::AVAILABLE; }

    void markOccupied() { status_ = ResourceStatus::OCCUPIED; }
    void markAvailable() { status_ = ResourceStatus::AVAILABLE; }
    void markMaintenance() { status_ = ResourceStatus::MAINTENANCE; }

    virtual std::string describe() const = 0;

protected:
    std::string resourceId_;
    ResourceType type_;
    std::string label_;
    ResourceStatus status_;
};

} // namespace agoms

#endif // AGOMS_RESOURCE_H
