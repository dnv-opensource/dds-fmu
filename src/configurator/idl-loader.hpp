#pragma once

#include <filesystem>
#include <xtypes/xtypes.hpp>
#include <xtypes/idl/idl.hpp>

eprosima::xtypes::idl::Context load_fmu_idls(const std::filesystem::path& fmu_root, const std::string& main_idl="dds-fmu.idl");
