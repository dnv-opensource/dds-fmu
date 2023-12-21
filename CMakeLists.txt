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
find_package(rapidxml REQUIRED CONFIG)
find_package(stduuid CONFIG REQUIRED)

find_package(zip CONFIG REQUIRED)
find_package(args CONFIG REQUIRED)

enable_testing()

# Target: link filesystem libraries for older compilers
add_library(filesystem::libs INTERFACE IMPORTED)
set_property(TARGET filesystem::libs
  PROPERTY INTERFACE_LINK_LIBRARIES
  $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.1>>:-lstdc++fs>
  $<$<AND:$<CXX_COMPILER_ID:Clang>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:-lc++fs>
  APPEND)

if(BUILD_TESTING)
  find_package(FMUComplianceChecker REQUIRED)
  find_package(GTest REQUIRED)
  add_subdirectory(tests)
endif()

cppfmu_prepare()
# Defines the following variables
#   FMU_UUID_DIR, FMU_OUTPUT_DIR,     FMU_STAGING_DIR,
#   CPPFMU_PLATFORM_ID, CPPFMU_CONFIG_STAGING_DIR
# Makes directory FMU_OUTPUT_DIR

set(fmuName dds-fmu)

set(fmuSourceFiles
  "${CMAKE_CURRENT_SOURCE_DIR}/src/dds-fmu/dds-fmu.cpp"
)

set(fmuModuleTarget "${fmuName}")
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

configure_file(
  "${CMAKE_SOURCE_DIR}/data/templates/modelDescription.xml.in"
  "${fmuIntermediateModelDescription}"
  @ONLY)

configure_file(
  "${CMAKE_SOURCE_DIR}/data/templates/config.hpp.in"
  "${CMAKE_BINARY_DIR}/temp/config.hpp"
  @ONLY)

# Target: GUID generation
add_custom_command(
  OUTPUT "${fmuGuidModelDescription}" ALWAYS
  COMMAND ${fmuRepackerTarget} "generate" ${fmuStagingDir}
  DEPENDS ${fmuIntermediateModelDescription} ${fmuStagingTarget}_1 ${fmuRepackerTarget}
  COMMENT "Generate GUID and its resources"
  VERBATIM
  )

add_custom_target(
  ${fmuGuidTarget}
  DEPENDS "${fmuGuidModelDescription}" ${fmuRepackerTarget}
  )

# Target: build module library
add_library(${fmuModuleTarget} MODULE
  ${fmuSourceFiles}
  ${CMAKE_BINARY_DIR}/cppfmu/fmi_functions.cpp
  )

target_link_libraries(${fmuModuleTarget}
  PUBLIC cppfmu::cppfmu fastdds::fastrtps
  PRIVATE configuration detail
  )

target_include_directories(${fmuModuleTarget}
  PRIVATE
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/temp>
  )

add_library(configuration OBJECT
  "${CMAKE_SOURCE_DIR}/src/configuration/auxiliaries.cpp"
  "${CMAKE_SOURCE_DIR}/src/configuration/model-descriptor.cpp"
  )

target_link_libraries(configuration
  PUBLIC
  eprosima::xtypes
  rapidxml::rapidxml
  PRIVATE
  filesystem::libs
  stduuid::stduuid)

target_include_directories(configuration
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/configuration>)


add_library(detail OBJECT
  ${CMAKE_SOURCE_DIR}/src/detail/Converter.cpp
  ${CMAKE_SOURCE_DIR}/src/detail/CustomKeyFilter.cpp
  ${CMAKE_SOURCE_DIR}/src/detail/DynamicPubSub.cpp
  ${CMAKE_SOURCE_DIR}/src/detail/SignalDistributor.cpp
  ${CMAKE_SOURCE_DIR}/src/detail/DataMapper.cpp
  )

target_link_libraries(detail
  PUBLIC
    eprosima::xtypes
    fastdds::fastrtps
    configuration
    cppfmu::cppfmu
  PRIVATE
    rapidxml::rapidxml
    filesystem::libs
  )

target_include_directories(detail
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/detail>)


add_executable(${fmuRepackerTarget} ${CMAKE_SOURCE_DIR}/src/repacker/repacker.cpp)

target_link_libraries(${fmuRepackerTarget}
  PRIVATE
  taywee::args
  zip::zip
  configuration
  detail
  filesystem::libs
  )

# Target: stage FMU contents phase 1
add_custom_target(${fmuStagingTarget}_1
  COMMAND "${CMAKE_COMMAND}" "-E" "make_directory" "${fmuStagedBinariesDir}"
  COMMAND "${CMAKE_COMMAND}" "-E" "make_directory" "${fmuStagedResourceBinariesDir}"
  COMMAND "${CMAKE_COMMAND}" "-E" "make_directory" "${fmuStagedResourceConfigDir}"
  COMMAND "${CMAKE_COMMAND}" "-E" "copy_directory" "${CMAKE_BINARY_DIR}/licenses" "${fmuStagingDir}/licenses"
  COMMAND "${CMAKE_COMMAND}" "-E" "copy_directory" "${CMAKE_SOURCE_DIR}/src/idl" "${fmuStagedResourceConfigDir}/idl"
  COMMAND "${CMAKE_COMMAND}" "-E" "copy_directory" "${CMAKE_SOURCE_DIR}/data/config" "${fmuStagedResourceConfigDir}/dds"
  COMMENT "FMU staging: Create and copy static FMU contents in staging directory"
  VERBATIM
  )

# Target: stage FMU contents phase 2
add_custom_target(${fmuStagingTarget}_2
  COMMAND "${CMAKE_COMMAND}" "-E" "copy" "$<TARGET_FILE:${fmuModuleTarget}>" "${fmuStagedBinariesDir}/"
  COMMAND "${CMAKE_COMMAND}" "-E" "copy" "$<TARGET_FILE:${fmuRepackerTarget}>" "${fmuStagedResourceBinariesDir}/"
  DEPENDS ${fmuModuleTarget} ${fmuRepackerTarget} ${fmuGuidTarget}
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
  COMMAND "${fmuRepackerTarget}" "zip" "-v" "-f" "--output=${fmuOutputFile}" "${fmuStagingDir}"
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


if(DDSFMU_WITH_DOC)
  add_subdirectory(docs)
endif()

set(ignored ${CMAKE_POLICY_DEFAULT_CMP0091})

source_group("Metadata" REGULAR_EXPRESSION "modelDescription.xml")
set_target_properties(${fmuGuidTarget} PROPERTIES FOLDER ${fmuName})
set_target_properties(${fmuModuleTarget} PROPERTIES PROJECT_LABEL "${fmuName}-lib")
set_target_properties(${fmuModuleTarget} PROPERTIES FOLDER ${fmuName})
set_target_properties(${fmuRepackerTarget} PROPERTIES FOLDER ${fmuName})
set_target_properties(${fmuStagingTarget}_1 PROPERTIES FOLDER ${fmuName})
set_target_properties(${fmuStagingTarget}_2 PROPERTIES FOLDER ${fmuName})
set_target_properties(${fmuArchiveTarget} PROPERTIES FOLDER ${fmuName})
