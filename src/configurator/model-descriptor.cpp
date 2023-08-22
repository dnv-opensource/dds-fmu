#include "model-descriptor.hpp"

#include <fstream>
#include <iostream>
#include <vector>

#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_print.hpp>

std::string print_xml(const rapidxml::xml_document<>& doc){
  constexpr int rapidxml_parse_flags = rapidxml::parse_full | rapidxml::parse_normalize_whitespace;
  std::string out_str;
  rapidxml::print(std::back_inserter(out_str), doc, rapidxml_parse_flags);
  return out_str;
}


void load_template_xml(rapidxml::xml_document<>& doc, const std::filesystem::path& template_xml, std::vector<char>& buffer){

  rapidxml::xml_node<> * root_node;

  std::ifstream theFile(template_xml.string());
  buffer = std::vector<char>(std::istreambuf_iterator<char>{theFile}, {});
  theFile.close();
  buffer.push_back('\0');

  constexpr int rapidxml_parse_flags = rapidxml::parse_full | rapidxml::parse_normalize_whitespace;

  doc.parse<rapidxml_parse_flags>(&buffer[0]);

  root_node = doc.first_node("fmiModelDescription");

  std::cout << "FMI version: " << root_node->first_attribute("fmiVersion")->value() << std::endl;
  std::cout << "Model name:  " << root_node->first_attribute("modelName")->value() << std::endl;
  std::cout << "Description: " << root_node->first_attribute("description")->value() << std::endl;
  std::cout << "Author:      " << root_node->first_attribute("author")->value() << std::endl;
  std::cout << "Copyright:   " << root_node->first_attribute("copyright")->value() << std::endl;
  std::cout << "License:     " << root_node->first_attribute("license")->value() << std::endl;
  std::cout << "Version:     " << root_node->first_attribute("version")->value() << std::endl;
  std::cout << "Guid:        " << root_node->first_attribute("guid")->value() << std::endl;
  std::cout << "Convention:  " << root_node->first_attribute("variableNamingConvention")->value() << std::endl;

}

void load_ddsfmu_mapping(rapidxml::xml_document<>& doc, const std::filesystem::path& ddsfmu_mapping, std::vector<char>& buffer){
  rapidxml::xml_node<> * root_node;
  std::ifstream theFile(ddsfmu_mapping.string());
  buffer = std::vector<char>(std::istreambuf_iterator<char>{theFile}, {});
  theFile.close();
  buffer.push_back('\0');
  doc.parse<0>(&buffer[0]);
}

void write_model_description(const rapidxml::xml_document<>& doc, const std::filesystem::path& fmu_root){

  auto data = print_xml(doc);
  auto filename = fmu_root / "modelDescription.xml";
  std::ofstream file;
  file.open(filename.c_str());
  file << data;
  file.close();

}



void name_generator(std::string& name, const eprosima::xtypes::DynamicData::ReadableNode& rnode)
{
  std::string member_name;
  bool is_array = rnode.type().kind() == eprosima::xtypes::TypeKind::ARRAY_TYPE;

  std::string from_name = (rnode.from_member() ? rnode.from_member()->name() : std::string());
  /*std::cout << "[" << rnode.type().name() << "]: ";
    << "Member name: " << from_name  << "\n";
    << "From member: " << rnode.from_member() << "\n"
    << "From index:  " << rnode.from_index() << "\n"
    << "Has parent:  " << rnode.has_parent() << "\n"
    << "Deep:        " << rnode.deep() << "\n";
  */

  // Determine name to give:
  if (rnode.from_member()) {
    member_name = rnode.from_member()->name();
  }

  // Parent is array
  if (rnode.has_parent() && rnode.parent().type().kind()
   == eprosima::xtypes::TypeKind::ARRAY_TYPE) {
    member_name += "[" + std::to_string(rnode.from_index()) + "]";
  }

  // Member name is non-empty and not an array
  if (!member_name.empty() && !is_array) {
    member_name += ".";
  }

  name.insert(0, member_name);

  if (rnode.has_parent()) {
    name_generator(name, rnode.parent());
  } else {
    if (name.back() == '.') {
      name.pop_back(); // remove trailing '.'
    }
  }
}


void model_variable_generator(
    rapidxml::xml_document<>& doc,
    rapidxml::xml_node<>* parent,
    const std::string& name,
    const std::string& causality,
    const std::uint32_t& value_ref,
    const ddsfmu::ScalarVariableType& type){

  char *name_val = doc.allocate_string(name.c_str());
  char *ref_val = doc.allocate_string(std::to_string(value_ref).c_str());
  char *caus_val = doc.allocate_string(causality.c_str());

  rapidxml::xml_node<>* node = doc.allocate_node(rapidxml::node_element, "ScalarVariable");
  rapidxml::xml_attribute<>* name_attr = doc.allocate_attribute("name", name_val);
  rapidxml::xml_attribute<>* ref_attr = doc.allocate_attribute("valueReference", ref_val);
  rapidxml::xml_attribute<>* variability_attr = doc.allocate_attribute("variability", "discrete");
  rapidxml::xml_attribute<>* causality_attr = doc.allocate_attribute("causality", caus_val);
  rapidxml::xml_attribute<>* initial_attr = doc.allocate_attribute("initial", "exact");

  node->append_attribute(name_attr);
  node->append_attribute(ref_attr);
  node->append_attribute(variability_attr);
  node->append_attribute(causality_attr);

  if (causality == "output") {
    node->append_attribute(initial_attr);
  }

  rapidxml::xml_node<>* child = nullptr;
  rapidxml::xml_attribute<>* start = nullptr;

  switch (type) {
  case ddsfmu::Real:
    {
      child = doc.allocate_node(rapidxml::node_element, "Real");
      start = doc.allocate_attribute("start", "0.0");
      break;
    }
  case ddsfmu::Integer:
    {
      child = doc.allocate_node(rapidxml::node_element, "Integer");
      start = doc.allocate_attribute("start", "0");
      break;
    }
  case ddsfmu::Boolean:
    {
      child = doc.allocate_node(rapidxml::node_element, "Boolean");
      start = doc.allocate_attribute("start", "false");
    break;
  }
  case ddsfmu::String:
  {
    child = doc.allocate_node(rapidxml::node_element, "String");
    start = doc.allocate_attribute("start", "");
    break;
  }
  default:
   break;
  }

  if (child && start) {
    child->append_attribute(start);
    node->insert_node(0, child);
  }
  parent->append_node(node);

}

void model_structure_outputs_generator(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* root, const std::uint32_t& num_outputs){

  rapidxml::xml_node<>* ms_node = doc.allocate_node(rapidxml::node_element, "ModelStructure");
  rapidxml::xml_node<>* out_node = doc.allocate_node(rapidxml::node_element, "Outputs");

  ms_node->insert_node(0, out_node);

  for (std::uint32_t i=1; i <= num_outputs; ++i){
    rapidxml::xml_node<>* unknown = doc.allocate_node(rapidxml::node_element, "Unknown");
    char *index_val = doc.allocate_string(std::to_string(i).c_str());
    rapidxml::xml_attribute<>* index = doc.allocate_attribute("index", index_val);
    unknown->append_attribute(index);
    out_node->insert_node(0, unknown);
  }
  root->append_node(ms_node);

}
