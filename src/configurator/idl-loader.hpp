#pragma once

#include <filesystem>
#include <xtypes/xtypes.hpp>
#include <xtypes/idl/idl.hpp>

/**
   @brief Load idl file and parse into xtypes context

   Loads the idl file, assumed to be located at
   "<resource_path>/config/idl/<main_idl>" and parses it into an xtypes context.

   @param [in] resource_path Resource directory of the FMU: "<fmu_root>/resources"
   @param [in] print Whether to print parsing
   @param [in] main_idl Name of the main idl file to load


*/
eprosima::xtypes::idl::Context load_fmu_idls(const std::filesystem::path& resource_path, bool print=false, const std::string& main_idl="dds-fmu.idl");
