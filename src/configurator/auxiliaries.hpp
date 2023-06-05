#pragma once
#include <string>
#include <vector>
#include <filesystem>

std::string generate_uuid(const std::vector<std::filesystem::path>& uuid_files, const std::vector<std::string>& strings=std::vector<std::string>());

std::vector<std::filesystem::path> get_uuid_files(const std::filesystem::path& fmu_root, bool skip_modelDescription=true /* TODO: false */);
