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
  using namespace std;
  uuids::uuid_name_generator generator(uuids::uuid::from_string(namespace_uuid).value());
  string to_uuid;

  // Copy file contents into buffer
  for (const fs::path& item : uuid_files){
    if(!fs::is_regular_file(item)){
      cerr << "File does not exist, skipping: " << item << endl;
      continue;
    }
    ifstream input_buffer(item.string());
    copy(istreambuf_iterator<char>{input_buffer}, {}, back_inserter(to_uuid));
  }

  // Copy extra strings into buffer
  for (const std::string& in_str : strings){
    copy(in_str.begin(), in_str.end(), back_inserter(to_uuid));
  }

  // Strips all CR LF and the section: guid="<uuid>" in modelDescription.xml
  std::regex strip_re("\r|\n|guid *= *\"[-0-9a-f]{36}\"|guid *= *\".*\"");
  // Generate uuid from filtered buffer contents
  return uuids::to_string(generator(regex_replace(to_uuid, strip_re, "")));
}


std::vector<std::filesystem::path> get_uuid_files(const std::filesystem::path& fmu_root, bool skip_modelDescription){

  auto idl_path = fmu_root / "resources" / "config";
  std::set<std::string> exts {".idl", ".xml", ".yml"};
  std::vector<fs::path> uuid_files;

  if (!skip_modelDescription){
    uuid_files.emplace_back(fmu_root / "modelDescription.xml");
    // std::cout << added: " << fmu_root / "modelDescription.xml" << std::endl;
  }

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

  return uuid_files;
}
