#include "idl-loader.hpp"

#include <stdexcept>

eprosima::xtypes::idl::Context load_fmu_idls(const std::filesystem::path& resource_path, bool print, const std::string& main_idl){

  namespace fs = std::filesystem;
  namespace ex = eprosima::xtypes;

  ex::idl::Context context;

  auto idl_dir = resource_path / "config" / "idl";
  auto entry_idl = idl_dir / main_idl;

  if(!fs::exists(entry_idl) && !fs::is_regular_file(entry_idl)){
    std::cerr << "Main idl file does not exist: "<< entry_idl << std::endl;
    throw std::runtime_error("Could not find IDL file: " + entry_idl.string());
  }

  context.log_level(ex::idl::log::LogLevel::xWARNING); // DEBUG
  context.print_log(true);
  context.include_paths.push_back(idl_dir.string());
  context = ex::idl::parse_file((entry_idl).string(), context);

  std::cout << "IDL parsing " << (context.success ? "Successful" : "Failed!") << std::endl;

  if (print) {
    for (auto [name, type] : context.get_all_scoped_types())
    {
      // Support printing all kinds
      if (type->kind() == ex::TypeKind::STRUCTURE_TYPE)
      {
        std::cout << "Struct Name:" << name << std::endl;
        auto members = static_cast<const ex::StructType*>(type.get())->members();
        for (auto &m: members)
        {
          if (m.type().kind() == ex::TypeKind::ALIAS_TYPE)
          {
            auto alias = static_cast<const ex::AliasType&>(m.type());
            std::cout << "Struct Member:" << name << "[" << m.name() << "," << alias.rget().name() << "]" << std::endl;
          }
          else
          {
            std::cout << "Struct Member:" << name << "[" << m.name() << "," << m.type().name() << "]" << std::endl;
          }
        }
      }
    }
  }
  return context;
}
