#include <iostream>
#include <filesystem>

#include "idl-loader.hpp"


int main(int argc, char **argv){

  namespace fs = std::filesystem;
  namespace ex = eprosima::xtypes;

  std::string filename("dds-fmu.idl");

  if(argc == 2) filename = std::string(argv[1]);

  auto context = load_fmu_idls(fs::current_path() / "fmu-staging" / "resources", true, filename);

  if(context.module().has_structure("my::Mind")){

    auto mind = context.module().structure("my::Mind");
    auto data_mind = eprosima::xtypes::DynamicData(mind);

    std::cout << std::endl << std::endl;

    for (eprosima::xtypes::ReadableDynamicDataRef::MemberPair&& elem : data_mind.items())
    {
      std::cout << elem.member().type().name() << " " << elem.member().name()
                << " is key: " << elem.member().is_key()
                << ", is optional: " << elem.member().is_optional() << std::endl;
    }
  }

  std::cout << "done" << std::endl;
  return 0;
}
