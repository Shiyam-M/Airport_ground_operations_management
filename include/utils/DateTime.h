#ifndef AGOMS_DATETIME_H
#define AGOMS_DATETIME_H

#include <string>
#include <ctime>

namespace agoms {
namespace util {

// A small, dependency-free wall-clock timestamp wrapper built on std::time_t.
// Kept intentionally simple for an academic project: enough to support
// ordering, interval overlap checks, and human-readable formatting.
class DateTime {
public:
    DateTime() : timestamp_(0) {}
    explicit DateTime(std::time_t t) : timestamp_(t) {}

    static DateTime now();

    // Build from "YYYY-MM-DD HH:MM" (24h clock). Returns epoch(0) on parse failure.
    static DateTime fromString(const std::string& text);

    // Add N minutes and return a new DateTime.
    DateTime addMinutes(int minutes) const;

    std::time_t raw() const { return timestamp_; }

    std::string toString() const;

    bool operator<(const DateTime& other) const { return timestamp_ < other.timestamp_; }
    bool operator>(const DateTime& other) const { return timestamp_ > other.timestamp_; }
    bool operator<=(const DateTime& other) const { return timestamp_ <= other.timestamp_; }
    bool operator>=(const DateTime& other) const { return timestamp_ >= other.timestamp_; }
    bool operator==(const DateTime& other) const { return timestamp_ == other.timestamp_; }
    bool operator!=(const DateTime& other) const { return timestamp_ != other.timestamp_; }

    // Difference in minutes (this - other).
    long minutesSince(const DateTime& other) const;

private:
    std::time_t timestamp_;
};

// Returns true if [aStart, aEnd) overlaps [bStart, bEnd).
bool timeWindowsOverlap(const DateTime& aStart, const DateTime& aEnd,
                         const DateTime& bStart, const DateTime& bEnd);

// Generates simple, human-friendly sequential IDs such as "AL0001", "GT0007".
class IdGenerator {
public:
    explicit IdGenerator(std::string prefix, int start = 1)
        : prefix_(std::move(prefix)), counter_(start) {}

    std::string next();

    // Ensures future generated IDs won't collide with an already-used numeric
    // suffix (used when re-hydrating counters from the database on startup).
    void observe(const std::string& existingId);

private:
    std::string prefix_;
    int counter_;
};

} // namespace util
} // namespace agoms

#endif // AGOMS_DATETIME_H
