#include "model-descriptor.hpp"

#include <iostream>

int main(int argc, char **argv)
{

  std::filesystem::path fmu_root = std::filesystem::current_path() / "fmu-staging";
  rapidxml::xml_document<> doc;
  std::vector<char> buffer_1;
  load_template_xml(doc, fmu_root / "resources" / "config" / "modelDescription.xml", buffer_1);

  std::cout << print_xml(doc) << std::endl;

  return 0;
}
