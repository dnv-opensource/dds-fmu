#include "model-descriptor.hpp"

#include <iostream>

int main(int argc, char **argv)
{

  std::filesystem::path fmu_root = std::filesystem::current_path() / "fmu-staging";

  std::cout << load_template_xml(fmu_root / "resources" / "config" / "modelDescription.xml") << std::endl;

  return 0;
}
