#pragma once

#include <xtypes/idl/idl.hpp>

#include <filesystem>

class DdsLoader
{
public:
  DdsLoader();
  bool load(const std::filesystem::path& resource_path);

private:
  bool load_profile(const std::filesystem::path& profile);
  bool m_loaded;
  eprosima::xtypes::idl::Context m_context;
};
