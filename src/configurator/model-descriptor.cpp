#include "model-descriptor.hpp"

#include <fstream>
#include <iostream>
#include <vector>

#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_print.hpp>


std::string load_template_xml(const std::filesystem::path& template_xml){

  rapidxml::xml_document<> doc;
  rapidxml::xml_node<> * root_node;

  std::ifstream theFile(template_xml.string());
  std::vector<char> buffer(std::istreambuf_iterator<char>{theFile}, {});
  theFile.close();
  buffer.push_back('\0');

  constexpr int rapidxml_parse_flags = rapidxml::parse_full | rapidxml::parse_normalize_whitespace;

  doc.parse<rapidxml_parse_flags>(&buffer[0]);

  root_node = doc.first_node("fmiModelDescription");

  //std::string myguid = "Not secret"; // allocate using rapidxml func if it needs to be alive
  //root_node->first_attribute("guid")->value(myguid.c_str());

  std::cout << root_node->first_attribute("fmiVersion")->value() << std::endl;
  std::cout << root_node->first_attribute("modelName")->value() << std::endl;
  std::cout << root_node->first_attribute("description")->value() << std::endl;
  std::cout << root_node->first_attribute("author")->value() << std::endl;
  std::cout << root_node->first_attribute("copyright")->value() << std::endl;
  std::cout << root_node->first_attribute("license")->value() << std::endl;
  std::cout << root_node->first_attribute("version")->value() << std::endl;
  std::cout << root_node->first_attribute("guid")->value() << std::endl;
  std::cout << root_node->first_attribute("variableNamingConvention")->value() << std::endl;

  std::string out_str;
  rapidxml::print(std::back_inserter(out_str), doc, rapidxml_parse_flags);

  return out_str;
}

// [x] load xml file into memory
// [x] how to add fields
// [x] dump intermediate xml tree to inmemory buffer (for uuid)
// acquired uuid -> add to guid field
// dump xml file to destination -> use ofstream
// std::ofstream file;
// file.open(filename.c_str());
// file << data;
// file.close();


std::string model_variable_generator(const std::string& name, const std::string& causality, const std::uint32_t& value_ref, const std::string& type){

  return std::string();
}

std::string model_structure_outputs_generator(const std::uint32_t& num_outputs){

  return std::string();
}
