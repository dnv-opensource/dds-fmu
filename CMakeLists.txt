cmake_minimum_required(VERSION 3.15)

file(STRINGS "${CMAKE_SOURCE_DIR}/version.txt" projectVersion)

project(dds-fmu
  VERSION "${projectVersion}"
  DESCRIPTION "DDS-FMU communication integration")

include(CTest)

option(DDSFMU_WITH_TOOLS "FMU re-packaging capabilities" ON)
option(DDSFMU_WITH_DOC "FMU documentation target" ON)
message(STATUS "dds-fmu: Option DDSFMU_WITH_TOOLS = ${DDSFMU_WITH_TOOLS}")

find_package(cppfmu CONFIG REQUIRED)
find_package(fastdds CONFIG REQUIRED)
find_package(xtypes CONFIG REQUIRED)
find_package(ryml CONFIG REQUIRED)
find_package(rapidxml REQUIRED CONFIG)
find_package(stduuid CONFIG REQUIRED)

find_package(zip CONFIG REQUIRED)
find_package(args CONFIG REQUIRED)

enable_testing()

if(BUILD_TESTING)
  find_package(FMUComplianceChecker REQUIRED)
  find_package(GTest REQUIRED)
  add_subdirectory(tests)
endif()

cppfmu_prepare()
# Defines the following variables
#   FMU_UUID_DIR, FMU_OUTPUT_DIR,     FMU_STAGING_DIR,
#   CPPFMU_ROOT,  CPPFMU_PLATFORM_ID, CPPFMU_CONFIG_STAGING_DIR
# Makes directory FMU_OUTPUT_DIR

set(fmuName dds-fmu)

set(fmuSourceFiles
  "${CMAKE_CURRENT_SOURCE_DIR}/src/dds-fmu/dds-fmu.cpp"
)

set(fmuModuleTarget "${fmuName}")
set(fmuDescriptorTarget "descriptor")
set(fmuRepackerTarget "repacker")
set(fmuGuidTarget "${fmuName}_guid")
set(fmuStagingTarget "${fmuName}_staging")
set(fmuArchiveTarget "${fmuName}_fmu")

set(fmuStagingDir "${CPPFMU_CONFIG_STAGING_DIR}")
set(fmuStagedBinariesDir "${fmuStagingDir}/binaries/${CPPFMU_PLATFORM_ID}")
set(fmuStagedResourceBinariesDir "${fmuStagingDir}/resources/tools/${CPPFMU_PLATFORM_ID}")
set(fmuStagedResourceConfigDir "${fmuStagingDir}/resources/config")
set(fmuOutputFile "${FMU_OUTPUT_DIR}/${fmuName}.fmu")

set(fmuGuidModelDescription "${fmuStagingDir}/modelDescription.xml")

# We suppose this file is generated from idl and xml parsing:
set(fmuIntermediateModelDescription "${fmuStagedResourceConfigDir}/modelDescription.xml")
set(fmuFinalModelDescription "${fmuStagingDir}/modelDescription.xml")

set(FMU_UUID "@FMU_UUID@") # Temporary workaround
configure_file(
  "${CMAKE_SOURCE_DIR}/data/templates/modelDescription.xml.in"
  "${fmuIntermediateModelDescription}"
  @ONLY)
unset(FMU_UUID)

configure_file(
  "${CMAKE_SOURCE_DIR}/data/templates/config.hpp.in"
  "${CMAKE_BINARY_DIR}/temp/config.hpp"
  @ONLY)

# Target: GUID generation
add_custom_command(
  OUTPUT "${fmuGuidModelDescription}" ALWAYS
  COMMAND ${fmuDescriptorTarget} ${fmuStagingDir}
  DEPENDS ${fmuIntermediateModelDescription} ${fmuStagingTarget}_1 ${fmuDescriptorTarget}
  COMMENT "Generate GUID and its resources"
  VERBATIM
  )

add_custom_target(
  ${fmuGuidTarget}
  DEPENDS "${fmuGuidModelDescription}" ${fmuDescriptorTarget}
  )

# Target: build module library
add_library(${fmuModuleTarget} MODULE
  ${fmuSourceFiles}
  ${CPPFMU_ROOT}/src/fmi_functions.cpp
  )

target_link_libraries(${fmuModuleTarget}
  PUBLIC cppfmu::cppfmu fastdds::fastrtps
  PRIVATE configurator helper # TODO add more here
  )

target_include_directories(${fmuModuleTarget}
  PRIVATE
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/temporary>
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/temp>
  )

add_executable(${fmuRepackerTarget}
  ${CMAKE_SOURCE_DIR}/src/repacker/repacker.cpp)

target_link_libraries(${fmuRepackerTarget}
  PRIVATE
  taywee::args
  zip::zip
  "$<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.1>>:-lstdc++fs>"
  "$<$<AND:$<CXX_COMPILER_ID:Clang>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:-lc++fs>"
  )

add_executable(${fmuDescriptorTarget}
  ${CMAKE_SOURCE_DIR}/src/descriptor/descriptor.cpp)

target_link_libraries(${fmuDescriptorTarget}
  PRIVATE
  taywee::args
  configurator
  helper
  "$<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.1>>:-lstdc++fs>"
  "$<$<AND:$<CXX_COMPILER_ID:Clang>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:-lc++fs>"
  )


# Target: stage FMU contents phase 1
add_custom_target(${fmuStagingTarget}_1
  COMMAND "${CMAKE_COMMAND}" "-E" "make_directory" "${fmuStagedBinariesDir}"
  COMMAND "${CMAKE_COMMAND}" "-E" "make_directory" "${fmuStagedResourceBinariesDir}"
  COMMAND "${CMAKE_COMMAND}" "-E" "make_directory" "${fmuStagedResourceConfigDir}"
  COMMAND "${CMAKE_COMMAND}" "-E" "copy_directory" "${CMAKE_SOURCE_DIR}/src/idl" "${fmuStagedResourceConfigDir}/idl"
  COMMAND "${CMAKE_COMMAND}" "-E" "copy_directory" "${CMAKE_SOURCE_DIR}/data/config" "${fmuStagedResourceConfigDir}/dds"
  COMMENT "FMU staging: Create and copy static FMU contents in staging directory"
  VERBATIM
  )

# Target: stage FMU contents phase 2
add_custom_target(${fmuStagingTarget}_2
  COMMAND "${CMAKE_COMMAND}" "-E" "copy" "$<TARGET_FILE:${fmuModuleTarget}>" "${fmuStagedBinariesDir}/"
  COMMAND "${CMAKE_COMMAND}" "-E" "copy" "$<TARGET_FILE:${fmuRepackerTarget}>" "${fmuStagedResourceBinariesDir}/"
  COMMAND "${CMAKE_COMMAND}" "-E" "copy" "$<TARGET_FILE:${fmuDescriptorTarget}>" "${fmuStagedResourceBinariesDir}/"
  DEPENDS ${fmuModuleTarget} ${fmuRepackerTarget} ${fmuDescriptorTarget} ${fmuGuidTarget}
  COMMENT "FMU staging: Populate FMU contents in staging directory"
  VERBATIM
  )


if(NOT DDSFMU_WITH_TOOLS)

  if(CMAKE_VERSION VERSION_LESS 3.17)
    set(_cmake_rmdir remove_directory)
    set(_cmake_rmdirflags -f)
  else()
    set(_cmake_rmdir rm)
    set(_cmake_rmdirflags -Rf)
  endif()

  add_custom_target(without_tools ALL
    COMMAND "${CMAKE_COMMAND}" "-E" "${_cmake_rmdir}" "${_cmake_rmdirflags}" "${fmuStagedResourceBinariesDir}"
    DEPENDS ${fmuStagingTarget}_2
    COMMENT "FMU without tools: Removing resources/tools/* from staging directory"
    VERBATIM)

endif()

# Target: generate FMU archive
add_custom_target(${fmuArchiveTarget} ALL
  COMMAND "${fmuRepackerTarget}" "-v" "-f" "--output=${fmuOutputFile}" "${fmuStagingDir}"
  DEPENDS ${fmuStagingTarget}_2 ${fmuModuleTarget} ${fmuRepackerTarget} ${fmuGuidTarget}
  COMMENT "Creating ${fmuName}.fmu archive"
  VERBATIM)

if(NOT DDSFMU_WITH_TOOLS)
add_dependencies(${fmuArchiveTarget} without_tools)
endif()

# Test compliance
if(BUILD_TESTING)
    add_fmu_compliance_check("'${fmuName}'_is_compliant" "${fmuOutputFile}")
endif()

source_group("Metadata" REGULAR_EXPRESSION "modelDescription.xml")
set_target_properties(${fmuGuidTarget} PROPERTIES FOLDER ${fmuName})
set_target_properties(${fmuModuleTarget} PROPERTIES PROJECT_LABEL "${fmuName}-lib")
set_target_properties(${fmuModuleTarget} PROPERTIES FOLDER ${fmuName})
set_target_properties(${fmuRepackerTarget} PROPERTIES FOLDER ${fmuName})
set_target_properties(${fmuStagingTarget}_1 PROPERTIES FOLDER ${fmuName})
set_target_properties(${fmuStagingTarget}_2 PROPERTIES FOLDER ${fmuName})
set_target_properties(${fmuArchiveTarget} PROPERTIES FOLDER ${fmuName})


add_library(helper OBJECT
  ${CMAKE_SOURCE_DIR}/src/helper/Converter.cpp
  ${CMAKE_SOURCE_DIR}/src/helper/DynamicPubSub.cpp
  ${CMAKE_SOURCE_DIR}/src/helper/DynamicPublisher.cpp
  ${CMAKE_SOURCE_DIR}/src/helper/DynamicSubscriber.cpp
  ${CMAKE_SOURCE_DIR}/src/helper/DdsLoader.cpp
  ${CMAKE_SOURCE_DIR}/src/helper/SignalDistributor.cpp
  ${CMAKE_SOURCE_DIR}/src/helper/DataMapper.cpp
  )

target_link_libraries(helper
  PUBLIC
    eprosima::xtypes
    fastdds::fastrtps
    configurator
  PRIVATE
    rapidxml::rapidxml
    "$<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.1>>:-lstdc++fs>"
    "$<$<AND:$<CXX_COMPILER_ID:Clang>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:-lc++fs>"
  )

target_include_directories(helper
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/helper>)


if(DDSFMU_WITH_DOC)
  add_subdirectory(docs)
endif()



# Temporary targets
add_library(configurator OBJECT
  "${CMAKE_SOURCE_DIR}/src/configurator/auxiliaries.cpp"
  "${CMAKE_SOURCE_DIR}/src/configurator/idl-loader.cpp"
  "${CMAKE_SOURCE_DIR}/src/configurator/model-descriptor.cpp"
  )

target_link_libraries(configurator
  PUBLIC
  eprosima::xtypes
  rapidxml::rapidxml
  PRIVATE
  "$<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.1>>:-lstdc++fs>"
  "$<$<AND:$<CXX_COMPILER_ID:Clang>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:-lc++fs>"
  stduuid::stduuid)

target_include_directories(configurator
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/configurator>)


add_library(trigs OBJECT
  "${CMAKE_CURRENT_SOURCE_DIR}/src/temporary/trig/trig.cxx"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/temporary/trig/trigPubSubTypes.cxx"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/temporary/trig/TrigPublisher.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/temporary/trig/TrigSubscriber.cpp"
)

target_link_libraries(trigs fastdds::fastrtps)

target_include_directories(trigs
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/temporary/trig>)

add_executable(myuuid ${CMAKE_SOURCE_DIR}/src/temporary/executables/myuuid.cpp)
target_link_libraries(myuuid configurator taywee::args)

add_executable(trigga "${CMAKE_CURRENT_SOURCE_DIR}/src/temporary/executables/trigga.cpp")
target_include_directories(trigga PRIVATE $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/temp>)
target_link_libraries(trigga trigs fastdds::fastrtps)

add_executable(xplora "${CMAKE_CURRENT_SOURCE_DIR}/src/temporary/executables/xplora.cpp")
target_link_libraries(xplora configurator)

add_executable(modeler "${CMAKE_CURRENT_SOURCE_DIR}/src/temporary/executables/modeler.cpp")
target_link_libraries(modeler configurator)

add_executable(pubdyntop "${CMAKE_SOURCE_DIR}/src/temporary/executables/pubdyntop.cpp")
target_link_libraries(pubdyntop fastdds::fastrtps rapidxml::rapidxml)

add_executable(subdyntop "${CMAKE_SOURCE_DIR}/src/temporary/executables/subdyntop.cpp")
target_link_libraries(subdyntop fastdds::fastrtps rapidxml::rapidxml)

add_executable(structuriser "${CMAKE_SOURCE_DIR}/src/temporary/executables/structuriser.cpp")
target_link_libraries(structuriser configurator)
