#include <iostream>
#include <filesystem>

#include "idl-loader.hpp"


int main(){

  namespace fs = std::filesystem;
  namespace ex = eprosima::xtypes;

  auto context = load_fmu_idls(fs::current_path() / "fmu-staging" / "resources", true, "dds-fmu.idl");

  std::cout << "done" << std::endl;
  return 0;
}
