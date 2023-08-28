
include(GoogleTest)

add_executable(unit-tests RunTests.cpp
  dynamic_pubsub.cpp
  model_description.cpp
  pubsub_procedure.cpp
  visitors.cpp
  xtypes.cpp
)

target_include_directories(unit-tests
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_link_libraries(unit-tests
  GTest::GTest
  eprosima::xtypes
  fastdds::fastrtps
  detail
  configurator
  )

gtest_discover_tests(unit-tests
  PROPERTIES ENVIRONMENT MY_VARIABLE=something
  TEST_SUFFIX "_$<CONFIGURATION>"
  EXTRA_ARGS "--gtest_output=xml:${CMAKE_BINARY_DIR}/testoutput")


add_custom_target(test-resources
  COMMAND "${CMAKE_COMMAND}" "-E" "copy_directory" "${CMAKE_SOURCE_DIR}/data/test_resources" "${CMAKE_CURRENT_BINARY_DIR}/resources"
  COMMENT "Copy test resources to binary test directory"
  VERBATIM
  )

add_dependencies(unit-tests test-resources)
