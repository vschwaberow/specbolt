find_package(Catch2 REQUIRED)

add_executable(specbolt_test SanityTest.cpp)
target_link_libraries(specbolt_test Catch2::Catch2WithMain)

add_test(NAME test COMMAND specbolt_test)