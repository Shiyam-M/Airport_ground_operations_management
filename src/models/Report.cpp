#include "models/Report.h"
#include <sstream>

namespace agoms {

Report::Report(std::string reportId, ReportType type, std::string title)
    : reportId_(std::move(reportId)), type_(type), title_(std::move(title)),
      generatedAt_(util::DateTime::now()) {}

std::string Report::render() const {
    std::ostringstream oss;
    oss << "==================================================\n";
    oss << " " << title_ << "\n";
    oss << " Report ID: " << reportId_ << " | Generated: " << generatedAt_.toString() << "\n";
    oss << "==================================================\n";
    for (const auto& line : lines_) {
        oss << line << "\n";
    }
    oss << "==================================================\n";
    return oss.str();
}

} // namespace agoms
