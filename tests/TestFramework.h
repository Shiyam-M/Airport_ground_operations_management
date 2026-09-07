#ifndef AGOMS_TEST_FRAMEWORK_H
#define AGOMS_TEST_FRAMEWORK_H

// A tiny, dependency-free unit test framework.
//
// GoogleTest was intentionally NOT pulled in via CMake FetchContent because
// this project needs to build in fully offline/sandboxed environments with
// no internet access. This header provides just enough of a TEST()/ASSERT_*
// style API to write clear, self-documenting tests. If GoogleTest is
// available in your environment and you prefer it, porting these test_*.cpp
// files to gtest's TEST() + EXPECT_*/ASSERT_* macros is straightforward
// since the structure (one function per test, assertions inside) matches.

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <sstream>

namespace agoms_test {

struct TestCase {
    std::string name;
    std::function<void()> fn;
};

// A test failure is signalled by throwing this exception; the test runner
// catches it, records the failure, and continues with the next test.
struct AssertionFailure {
    std::string message;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

struct Registrar {
    Registrar(const std::string& name, std::function<void()> fn) {
        registry().push_back({name, std::move(fn)});
    }
};

inline int runAllTests() {
    int passed = 0, failed = 0;
    for (auto& test : registry()) {
        try {
            test.fn();
            std::cout << "[ PASS ] " << test.name << "\n";
            ++passed;
        } catch (const AssertionFailure& failure) {
            std::cout << "[ FAIL ] " << test.name << " -- " << failure.message << "\n";
            ++failed;
        } catch (const std::exception& ex) {
            std::cout << "[ FAIL ] " << test.name << " -- unexpected exception: " << ex.what() << "\n";
            ++failed;
        } catch (...) {
            std::cout << "[ FAIL ] " << test.name << " -- unknown exception\n";
            ++failed;
        }
    }
    std::cout << "\n=====================================\n";
    std::cout << "Total: " << (passed + failed) << " | Passed: " << passed << " | Failed: " << failed << "\n";
    std::cout << "=====================================\n";
    return failed == 0 ? 0 : 1;
}

} // namespace agoms_test

#define AGOMS_CONCAT_INNER(a, b) a##b
#define AGOMS_CONCAT(a, b) AGOMS_CONCAT_INNER(a, b)

#define TEST(name) \
    void AGOMS_CONCAT(agoms_test_fn_, name)(); \
    static agoms_test::Registrar AGOMS_CONCAT(agoms_test_registrar_, name)(#name, AGOMS_CONCAT(agoms_test_fn_, name)); \
    void AGOMS_CONCAT(agoms_test_fn_, name)()

#define ASSERT_TRUE(cond) \
    do { if (!(cond)) throw agoms_test::AssertionFailure{std::string("ASSERT_TRUE failed: ") + #cond + \
        " (" __FILE__ ":" + std::to_string(__LINE__) + ")"}; } while (0)

#define ASSERT_FALSE(cond) ASSERT_TRUE(!(cond))

#define ASSERT_EQ(expected, actual) \
    do { \
        auto _e = (expected); auto _a = (actual); \
        if (!(_e == _a)) { \
            std::ostringstream _oss; \
            _oss << "ASSERT_EQ failed: expected [" << _e << "] but got [" << _a << "] (" \
                 << __FILE__ << ":" << __LINE__ << ")"; \
            throw agoms_test::AssertionFailure{_oss.str()}; \
        } \
    } while (0)

#define ASSERT_THROWS(expr, exType) \
    do { \
        bool _threw = false; \
        try { expr; } catch (const exType&) { _threw = true; } \
        catch (...) { throw agoms_test::AssertionFailure{std::string("ASSERT_THROWS: wrong exception type (") + \
            __FILE__ ":" + std::to_string(__LINE__) + ")"}; } \
        if (!_threw) throw agoms_test::AssertionFailure{std::string("ASSERT_THROWS: no exception thrown (") + \
            __FILE__ ":" + std::to_string(__LINE__) + ")"}; \
    } while (0)

#endif // AGOMS_TEST_FRAMEWORK_H
