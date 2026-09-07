#ifndef AGOMS_GATE_H
#define AGOMS_GATE_H

#include "models/Resource.h"

namespace agoms {

class Gate : public Resource {
public:
    Gate(std::string resourceId, std::string terminal)
        : Resource(resourceId, ResourceType::GATE, resourceId),
          terminal_(std::move(terminal)) {}

    const std::string& getTerminal() const { return terminal_; }

    std::string describe() const override {
        return "Gate " + resourceId_ + " (Terminal " + terminal_ + ") - " + toString(status_);
    }

private:
    std::string terminal_;
};

} // namespace agoms

#endif // AGOMS_GATE_H
