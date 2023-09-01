#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <rapidxml/rapidxml.hpp>
#include <xtypes/idl/idl.hpp>



namespace ddsfmu {
namespace config {

/**
   @brief Finds structured name for a given node

   @param [out] name Variable name of node according to FMU structured naming convention
   @param [in] rnode Node to inspect

   Internally, this function recursively calls itself to find all ancestors of a node.
   The convention used is '.' for members and '[i]', with i being zero-indexed array notation.

*/
void name_generator(std::string& name, const eprosima::xtypes::DynamicData::ReadableNode& rnode);


// TODO: for fmi3 this and related impl need to be extended.
/// Primitive type kinds in FMI
enum class ScalarVariableType { Real, Integer, Boolean, String, Unknown };


/**
   @brief Prints xml document as string

   @return XML file loaded as a string

*/
std::string print_xml(const rapidxml::xml_document<>& doc);

/**
   @brief Loads template modelDescription.xml

   Loads the modelDescription template file whose contents is independent from the signal
   configuration, that is, before adding information from IDL and other configuration files.

   @param [in, out] doc XML document being loaded
   @param [in] template_xml Path to modelDescription.xml
   @param [in, out] buffer Buffer that the function uses to load file contents

*/
void load_template_xml(
  rapidxml::xml_document<>& doc, const std::filesystem::path& template_xml,
  std::vector<char>& buffer);

/**
   @brief Loads mapping between dds topics and fmu signal references ddsfmu_mapping

   Loads the mapping defined in ddsfmu_mapping file

   @param [in, out] doc XML document being loaded
   @param [in] ddsfmu_mapping Path to ddsfmu_mapping XML file
   @param [in, out] buffer Buffer that the function uses to load file contents

*/
void load_ddsfmu_mapping(
  rapidxml::xml_document<>& doc, const std::filesystem::path& ddsfmu_mapping,
  std::vector<char>& buffer);

/**
   @brief Writes contents of xml document to modelDescription.xml

   The content is written to fmu_root / "modelDescription.xml", replacing the file if it exists.

   @param [in] doc XML document to be dumped to file
   @param [in] fmu_root Path to FMU root directory, where the file will be written

*/
void write_model_description(
  const rapidxml::xml_document<>& doc, const std::filesystem::path& fmu_root);

/**
   @brief Constructs a `<ScalarVariable>` tag to be used in `<ModelVariables>` of the modelDescription.xml

   @param [in,out] doc XML document to work on
   @param [in,out] model_variables_node ModelVariables node to be appended
   @param [in] name Name attribute
   @param [in] causality Causality attribute input|output
   @param [in] value_ref valueReference attribute
   @param [in] type ScalarVariable type: Real|Integer|Boolean|String

*/
void model_variable_generator(
  rapidxml::xml_document<>& doc, rapidxml::xml_node<>* model_variables_node,
  const std::string& name, const std::string& causality, const std::uint32_t& value_ref,
  const ddsfmu::config::ScalarVariableType& type);

/**
   @brief Create `<Outputs>` tag with necessary `<Unknown>`

   Our implementation puts all outputs before inputs, so inferring indices are
   straightforward, namely one-indexed with the number of outputs, starting with 1.

   @param [in, out] doc XML document to work on
   @param [in, out] root Node for which to attach `<ModelStructure>`
   @param [in] num_outputs Number of outputs
*/
void model_structure_outputs_generator(
  rapidxml::xml_document<>& doc, rapidxml::xml_node<>* root, const std::uint32_t& num_outputs);

}
}
