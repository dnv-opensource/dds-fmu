#include "DdsLoader.hpp"

#include <filesystem>
#include <iostream>

#include <fastrtps/xmlparser/XMLProfileManager.h>

#include "idl-loader.hpp"

DdsLoader::DdsLoader(): m_loaded(false) { }

bool DdsLoader::load(const std::filesystem::path& resource_path){

  if (m_loaded) { return true; }

  auto dds_profile = (resource_path / "config" / "dds" / "dds_profile.xml");

  if (!load_profile(dds_profile)) { return false; }

  try {
    m_context = load_fmu_idls(resource_path, false, "dds-fmu.idl");
  }
  catch (const std::runtime_error& e){
    std::cerr << e.what() << std::endl;
    return false;
  }
  m_loaded = true;
  return true;
}

bool DdsLoader::load_profile(const std::filesystem::path& dds_profile){

  if (eprosima::fastrtps::xmlparser::XMLP_ret::XML_OK !=
   eprosima::fastrtps::xmlparser::XMLProfileManager::loadXMLFile(dds_profile.string()))
  {
    std::cout << "Cannot load XML file " << dds_profile << std::endl;
    return false;
  }

  return true;
}
