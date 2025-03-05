find_package(GTest REQUIRED)

# Test executables
add_executable(unique_ptr_test unique_ptr_test.cpp)
add_executable(shared_ptr_test shared_ptr_test.cpp)
add_executable(weak_ptr_test weak_ptr_test.cpp)

# Link dependencies
target_link_libraries(unique_ptr_test PRIVATE smart_ptr_kit GTest::GTest GTest::Main)
target_link_libraries(shared_ptr_test PRIVATE smart_ptr_kit GTest::GTest GTest::Main)
target_link_libraries(weak_ptr_test PRIVATE smart_ptr_kit GTest::GTest GTest::Main)

# Register tests
add_test(NAME unique_ptr_test COMMAND unique_ptr_test)
add_test(NAME shared_ptr_test COMMAND shared_ptr_test)
add_test(NAME weak_ptr_test COMMAND weak_ptr_test)