// tests/test_main.cpp
// Entry point for the AGOMS test suite. All actual test cases are defined
// with the TEST(name) { ... } macro across the other test_*.cpp files in
// this directory; this file just runs the full registry.
#include "TestFramework.h"

int main() {
    return agoms_test::runAllTests();
}
