#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>
#include <string>
#include <string_view>
#include <regex>
#include <vector>

#include <uuid.h>
#include "auxiliaries.hpp"

namespace fs = std::filesystem;

std::string generate_uuid(const std::vector<std::filesystem::path>& uuid_files, const std::vector<std::string>& strings){

  constexpr std::string_view namespace_uuid = "1a9ff216-b23c-24a7-ff73-e4e6d3ab3dcd";
  uuids::uuid_name_generator generator(uuids::uuid::from_string(namespace_uuid).value());
  std::string to_uuid;

  // Copy file contents into buffer
  for (const fs::path& item : uuid_files){
    if(!fs::is_regular_file(item)){
      std::cerr << "File does not exist, skipping: " << item << std::endl;
      continue;
    }
    std::ifstream input_buffer(item.string());
    std::copy(std::istreambuf_iterator<char>{input_buffer}, {}, std::back_inserter(to_uuid));
  }

  // Copy extra strings into buffer
  for (const std::string& in_str : strings){
    //std::cout << in_str << std::endl;
    copy(in_str.begin(), in_str.end(), back_inserter(to_uuid));
  }

  // Strips all whitespace CR LF and the section: guid="<uuid>" in modelDescription.xml
  std::regex strip_re("\\s+|\r|\n|guid *= *\"[-0-9a-f]{36}\"|guid *= *\"[-0-9a-z]{36}\"");
  // Generate uuid from filtered buffer contents

  std::string contents = regex_replace(to_uuid, strip_re, "");
  //std::cout << contents << std::endl;
  return uuids::to_string(generator(contents));
}


std::vector<std::filesystem::path> get_uuid_files(const std::filesystem::path& fmu_root, bool skip_modelDescription){

  auto idl_path = fmu_root / "resources" / "config";
  std::set<std::string> exts {".idl", ".xml", ".yml"};
  std::vector<fs::path> uuid_files;

  if (!fs::exists(idl_path)) {
    std::cerr << "Expected path does not exist: " << idl_path << std::endl;
  } else {
    for (const fs::directory_entry& dir_entry : fs::recursive_directory_iterator(idl_path))
    {
      if (dir_entry.is_regular_file() && exts.count(dir_entry.path().extension()) != 0){
        uuid_files.emplace_back(dir_entry);
        //std::cout << "added: " << dir_entry << std::endl;
      }
    }
  }

  if (!skip_modelDescription){
    uuid_files.emplace_back(fmu_root / "modelDescription.xml");
    // std::cout << added: " << fmu_root / "modelDescription.xml" << std::endl;
  }

  return uuid_files;
}
