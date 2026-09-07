#include "utils/DateTime.h"
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <algorithm>
#include <cctype>

namespace agoms {
namespace util {

DateTime DateTime::now() {
    return DateTime(std::time(nullptr));
}

DateTime DateTime::fromString(const std::string& text) {
    std::tm tm{};
    std::istringstream ss(text);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M");
    if (ss.fail()) {
        return DateTime(static_cast<std::time_t>(0));
    }
    tm.tm_isdst = -1;
    std::time_t t = std::mktime(&tm);
    return DateTime(t);
}

DateTime DateTime::addMinutes(int minutes) const {
    return DateTime(timestamp_ + static_cast<std::time_t>(minutes) * 60);
}

std::string DateTime::toString() const {
    if (timestamp_ == 0) return "N/A";
    std::tm* tmPtr = std::localtime(&timestamp_);
    if (!tmPtr) return "N/A";
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", tmPtr);
    return std::string(buf);
}

long DateTime::minutesSince(const DateTime& other) const {
    return static_cast<long>((timestamp_ - other.timestamp_) / 60);
}

bool timeWindowsOverlap(const DateTime& aStart, const DateTime& aEnd,
                         const DateTime& bStart, const DateTime& bEnd) {
    return aStart < bEnd && bStart < aEnd;
}

std::string IdGenerator::next() {
    std::ostringstream oss;
    oss << prefix_ << std::setw(4) << std::setfill('0') << counter_;
    ++counter_;
    return oss.str();
}

void IdGenerator::observe(const std::string& existingId) {
    if (existingId.size() <= prefix_.size()) return;
    if (existingId.compare(0, prefix_.size(), prefix_) != 0) return;
    std::string numPart = existingId.substr(prefix_.size());
    if (numPart.empty() || !std::all_of(numPart.begin(), numPart.end(), ::isdigit)) return;
    int value = std::atoi(numPart.c_str());
    if (value >= counter_) counter_ = value + 1;
}

} // namespace util
} // namespace agoms
