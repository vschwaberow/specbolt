#include <catch2/catch_test_macros.hpp>

TEST_CASE("SanityTest") {
  SECTION("Some test") { CHECK(1 == 1); }
}
