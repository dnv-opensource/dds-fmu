/*
  Copyright 2023, SINTEF Ocean
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <filesystem>
#include <regex>
#include <stdexcept>
#include <string>

#include "FmuInstance.hpp"
#include "auxiliaries.hpp"

cppfmu::UniquePtr<cppfmu::SlaveInstance> CppfmuInstantiateSlave(
  cppfmu::FMIString instanceName, cppfmu::FMIString fmuGUID,
  cppfmu::FMIString fmuResourceLocation, cppfmu::FMIString /*mimeType*/,
  cppfmu::FMIReal /*timeout*/, cppfmu::FMIBoolean /*visible*/, cppfmu::FMIBoolean /*interactive*/,
  cppfmu::Memory memory, cppfmu::Logger logger) {

#ifdef _WIN32
  std::string file_rex("file:///");
#else
  std::string file_rex("file://");
#endif
  auto resource_dir =
    std::filesystem::path(std::regex_replace(fmuResourceLocation, std::regex(file_rex.c_str()), ""));

  auto fmu_base_path = resource_dir.parent_path();
  auto evalGUID =
    ddsfmu::config::generate_uuid(ddsfmu::config::get_uuid_files(fmu_base_path, true));

  if (evalGUID != std::string(fmuGUID)) {
    throw std::runtime_error(
      std::string("FMU GUID mismatch: Got from ModelDescription: ") + std::string(fmuGUID)
      + std::string(", but evaluated: ") + evalGUID);
  }

  return cppfmu::AllocateUnique<ddsfmu::FmuInstance>(memory, std::string(instanceName), resource_dir, logger);
}
