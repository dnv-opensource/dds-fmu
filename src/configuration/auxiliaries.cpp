/*
  Copyright 2023, SINTEF Ocean
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "auxiliaries.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <uuid.h>
#include <xtypes/xtypes.hpp>

namespace ddsfmu {
namespace config {

namespace fs = std::filesystem;

std::string generate_uuid(
  const std::vector<std::filesystem::path>& uuid_files, const std::vector<std::string>& strings) {
  constexpr std::string_view namespace_uuid = "1a9ff216-b23c-24a7-ff73-e4e6d3ab3dcd";
  uuids::uuid_name_generator generator(uuids::uuid::from_string(namespace_uuid).value());
  std::string to_uuid;

  // Copy file contents into buffer
  for (const fs::path& item : uuid_files) {
    if (!fs::is_regular_file(item)) {
      std::cerr << "File does not exist, skipping: " << item << std::endl;
      continue;
    }
    std::ifstream input_buffer(item.string());
    std::copy(std::istreambuf_iterator<char>{input_buffer}, {}, std::back_inserter(to_uuid));
  }

  // Copy extra strings into buffer
  for (const std::string& in_str : strings) {
    //std::cout << in_str << std::endl;
    std::copy(in_str.begin(), in_str.end(), back_inserter(to_uuid));
  }

  // Strips all whitespace CR LF and the section: guid="<uuid>" in modelDescription.xml
  std::regex strip_re("\\s+|\r|\n|guid *= *\"[-0-9a-f]{36}\"|guid *= *\"[-0-9a-z]{36}\"");
  // Generate uuid from filtered buffer contents

  std::string contents = std::regex_replace(to_uuid, strip_re, "");
  //std::cout << contents << std::endl;
  return uuids::to_string(generator(contents));
}


std::vector<std::filesystem::path>
  get_uuid_files(const std::filesystem::path& fmu_root, bool skip_modelDescription) {
  auto idl_path = fmu_root / "resources" / "config";
  std::set<std::string> exts{".idl", ".xml", ".yml"};
  std::vector<fs::path> uuid_files;

  if (!fs::exists(idl_path)) {
    std::cerr << "Expected path does not exist: " << idl_path << std::endl;
  } else {
    for (const fs::directory_entry& dir_entry : fs::recursive_directory_iterator(idl_path)) {
      if (dir_entry.is_regular_file() && exts.count(dir_entry.path().extension().string()) != 0) {
        uuid_files.emplace_back(dir_entry);
        //std::cout << "added: " << dir_entry << std::endl;
      }
    }
  }

  if (!skip_modelDescription) {
    uuid_files.emplace_back(fmu_root / "modelDescription.xml");
    // std::cout << added: " << fmu_root / "modelDescription.xml" << std::endl;
  }

  std::sort(uuid_files.begin(), uuid_files.end(),
          [](const std::filesystem::path& a, const std::filesystem::path& b) {
              return a.string() < b.string();
          });

  return uuid_files;
}

eprosima::xtypes::idl::Context load_fmu_idls(
  const std::filesystem::path& resource_path, bool print, const std::string& main_idl) {
  namespace fs = std::filesystem;
  namespace ex = eprosima::xtypes;

  ex::idl::Context context;

  auto idl_dir = resource_path / "config" / "idl";
  auto entry_idl = idl_dir / main_idl;

  if (!fs::exists(entry_idl) && !fs::is_regular_file(entry_idl)) {
    std::cerr << "Main idl file does not exist: " << entry_idl << std::endl;
    throw std::runtime_error("Could not find IDL file: " + entry_idl.string());
  }

  context.log_level(ex::idl::log::LogLevel::xDEBUG); // WARNING
  context.print_log(false);
  context.preprocess = false; // Requires a compiler preprocessor (gcc or cl)
  context.include_paths.push_back(idl_dir.string());
  context = ex::idl::parse_file((entry_idl).string(), context);

  //std::cout << "IDL parsing: " << (context.success ? "Successful" : "Failed!") << std::endl;

  if (!context.success) {
    std::ostringstream oss;
    oss << std::endl;
    for (auto& entry : context.log()) { oss << entry.to_string() << std::endl; }
    throw std::runtime_error("Failed to parse IDL files!" + oss.str());
  }

  if (print) {
    for (auto [name, type] : context.get_all_scoped_types()) {
      // TODO: Support printing all kinds
      if (type->kind() == ex::TypeKind::STRUCTURE_TYPE) {
        std::cout << "Struct Name:" << name << std::endl;
        auto members = static_cast<const ex::StructType*>(type.get())->members();
        for (auto& m : members) {
          if (m.type().kind() == ex::TypeKind::ALIAS_TYPE) {
            auto alias = static_cast<const ex::AliasType&>(m.type());
            std::cout << "Struct Member:" << name << "[" << m.name() << "," << alias.rget().name()
                      << "]" << std::endl;
          } else {
            std::cout << "Struct Member:" << name << "[" << m.name() << "," << m.type().name()
                      << "]" << std::endl;
          }
        }
      }
    }
  }
  return context;
}

}
}
