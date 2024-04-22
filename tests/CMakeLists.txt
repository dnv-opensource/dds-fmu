
include(GoogleTest)

add_executable(unit-tests RunTests.cpp
  dynamic_pubsub.cpp
  keyed_members.cpp
  model_description.cpp
  pubsub_procedure.cpp
  visitors.cpp
  xtypes.cpp
  hello_pubsub.cpp
)

add_executable(hello-test hello_main.cpp
  hello_pubsub.cpp)


if(MSVC)
  target_compile_options(unit-tests PRIVATE /bigobj)
  target_compile_options(hello-test PRIVATE /bigobj)
endif()


target_include_directories(hello-test
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_link_libraries(hello-test
  eprosima::xtypes
  fastdds::fastrtps
  detail
  configuration
  filesystem::libs
  )

target_include_directories(unit-tests
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_link_libraries(unit-tests
  GTest::GTest
  eprosima::xtypes
  fastdds::fastrtps
  detail
  configuration
  filesystem::libs
  )

gtest_discover_tests(unit-tests
  PROPERTIES ENVIRONMENT MY_VARIABLE=something
  TEST_SUFFIX "_$<CONFIG>"
  XML_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/testoutput/")

add_custom_target(test-resources
  COMMAND "${CMAKE_COMMAND}" "-E" "copy_directory" "${CMAKE_SOURCE_DIR}/data/test_resources" "${CMAKE_CURRENT_BINARY_DIR}/resources"
  COMMENT "Copy test resources to binary test directory"
  VERBATIM
  )

add_dependencies(unit-tests test-resources)
