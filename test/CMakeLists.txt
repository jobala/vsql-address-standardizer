include(AddGoogleTest)
add_executable(address_standardizer_test address_standardizer_test.cc)

target_link_libraries(address_standardizer_test PRIVATE GTest::gtest GTest::gtest_main address_standardizer)


include(GoogleTest)
gtest_discover_tests(address_standardizer_test DISCOVERY_MODE PRE_TEST)
