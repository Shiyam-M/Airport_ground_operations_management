#ifndef AGOMS_REPORT_H
#define AGOMS_REPORT_H

#include <string>
#include <vector>
#include "models/Enums.h"
#include "utils/DateTime.h"

namespace agoms {

// A generated report: a type, a title, and a set of pre-formatted text
// lines. Kept intentionally simple (no generic tabular model) since this is
// a console application; ReportService is responsible for populating the
// lines from the appropriate domain data.
class Report {
public:
    Report(std::string reportId, ReportType type, std::string title);

    const std::string& getReportId() const { return reportId_; }
    ReportType getType() const { return type_; }
    const std::string& getTitle() const { return title_; }
    util::DateTime getGeneratedAt() const { return generatedAt_; }

    void addLine(const std::string& line) { lines_.push_back(line); }
    const std::vector<std::string>& getLines() const { return lines_; }

    std::string render() const;

private:
    std::string reportId_;
    ReportType type_;
    std::string title_;
    util::DateTime generatedAt_;
    std::vector<std::string> lines_;
};

} // namespace agoms

#endif // AGOMS_REPORT_H
