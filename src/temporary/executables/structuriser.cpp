#include "idl-loader.hpp"
#include <iostream>
#include <filesystem>

#include <string>
#include <utility>
#include <iterator>
#include <algorithm>
#include <functional>
#include <list>

int main(){
  namespace fs = std::filesystem;
  namespace ex = eprosima::xtypes;
  auto my_context = load_fmu_idls(fs::current_path() / "fmu-staging" / "resources", false, "dds-fmu.idl");

  for (auto [name, type] : my_context.get_all_scoped_types()){
    std::cout << name << std::endl; // type: (DynamicType::Ptr)
  }

  // to be converted to an fmu signal
  auto topic_type = std::make_pair<std::string,std::string>("skjong", "Kong::Fu");

  const ex::DynamicType& fu = my_context.module().structure(topic_type.second);
  ex::DynamicData data(fu); // fu must outlive data (my_context by extension)
  //std::cout << fu.name() << std::endl;

  try {
    data["fighter"] = 3.14;
    data["looser"] = true;
    data["member"] = "Sund";
    data["my_array"][0][1] = std::uint32_t(1);
    data["my_array"][1][0] = std::uint32_t(10);
    data["my_array"][1][1] = std::uint32_t(11);
    data["my_array"][2][0] = std::uint32_t(20);
    data["my_array"][2][1] = std::uint32_t(21);
    data["onometry"]["sine"] = 2.7;
    data["onometry"]["other"] = -1.0;

    std::cout << data << std::endl;
  } catch(const std::runtime_error&e){
    std::cout << "Dynamic data allocation error" << e.what() << std::endl;
  }

  std::cout << data.type().name() << std::endl;

  for (ex::ReadableDynamicDataRef::MemberPair&& elem : data.items()) {

    if (elem.kind() == ex::TypeKind::STRING_TYPE) {
      std::cout << elem.member().type().name() << " " << elem.member().name()
                << ": " << elem.data().value<std::string>() << std::endl;
    } else if (elem.kind() == ex::TypeKind::STRUCTURE_TYPE) {
      std::cout << elem.member().type().name() << " " << elem.member().name() << std::endl;
    } else if (elem.kind() == ex::TypeKind::FLOAT_64_TYPE) {
      std::cout << elem.member().type().name() << " " << elem.member().name()
                << ": " << elem.data().value<double>() << std::endl;
    } else if (elem.kind() == ex::TypeKind::BOOLEAN_TYPE) {
      std::cout << elem.member().type().name() << " " << elem.member().name()
                << ": " << elem.data().value<bool>() << std::endl;
    }
  }

  data.for_each([&](const ex::DynamicData::ReadableNode& node)
  {
    auto member = node.from_member();
    std::string name("");
    if(member) name = member->name();

    switch(node.type().kind())
    {
    case ex::TypeKind::STRUCTURE_TYPE:
      std::cout << node.type().name() << " " << name;
      break;
    case ex::TypeKind::ARRAY_TYPE:
      std::cout << node.type().name() << " " << name << " Dimension: " << node.data().size() << ", idx: " << node.from_index();
      break;
    case ex::TypeKind::STRING_TYPE:
      std::cout << node.type().name() << " " << name << ": " << node.data().value<std::string>();
      break;
    case ex::TypeKind::FLOAT_64_TYPE:
      std::cout << node.type().name() << " " << name << ": " << node.data().value<double>();
      break;
    case ex::TypeKind::BOOLEAN_TYPE:
      std::cout << node.type().name() << " " << name << ": " << node.data().value<bool>();
      break;
    case ex::TypeKind::UINT_32_TYPE:
      std::cout << node.type().name() << " " << name << ": " << node.data().value<uint32_t>() << ", idx: " << node.from_index();
      break;
    }

  });

  // Note: Cannot support unbounded sequences or other unbounded

}
