#pragma once
#include <string>
#include <vector>
#include <filesystem>


/**
   @brief Creates a uuid from a list of files and list of strings

   This functions loads the contents of each listed file and appends the list of strings
   also provided by the user. Next, it strips spaces, CR, LF, as well as the section: guid="<uuid>"
   found in modelDescription.xml. The resulting string buffer is then used to create a
   uuid, which is returned as a string.

   @param [in] uuid_files List of files whose contents to be loaded
   @param [in] strings List of strings to be added
   @return Generated uuid string

*/
std::string generate_uuid(const std::vector<std::filesystem::path>& uuid_files, const std::vector<std::string>& strings=std::vector<std::string>());

/**
   @brief Given FMU root, returns a list of file paths to be used to generate uuid

   This function lists all files in "<fmu_root>/resources/config/" with file extensions [.xml, .idl, .yml].
   In addition, optionally add "<fmu_root>/modelDescription.xml".

   @param [in] fmu_root Root directory to search for files
   @param [in] skip_modelDescription Whether to skip model model description in the return list of paths

   @return List of paths to files.
*/
std::vector<std::filesystem::path> get_uuid_files(const std::filesystem::path& fmu_root, bool skip_modelDescription=true /* TODO: false */);
