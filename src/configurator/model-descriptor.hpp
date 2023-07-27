#pragma once
#include <filesystem>
#include <string>

/**
   @brief Loads template modelDescription.xml

   Loads the modelDescription template file whose contents is independent from the signal
   configuration, that is, before adding information from IDL and other configuration files.

   @param [in] template_xml Path to modelDescription.xml
   @return XML file loaded as a string

*/
std::string load_template_xml(const std::filesystem::path& template_xml);


/**
   @brief Constructs a <ScalarVariable> tag to be used in <ModelVariables> of the modelDescription.xml

   @param [in] name Name attribute
   @param [in] causality Causality attribute input|output
   @param [in] value_ref valueReference attribute
   @param [in] type ScalarVariable type: Real|Integer|Boolean|String
   // add start?
   @return <ScalarVariable><...></ScalarVariable> as string

*/
std::string model_variable_generator(const std::string& name, const std::string& causality, const std::uint32_t& value_ref, const std::string& type); // replace type with enum type Real Integer Boolean, String

/**
   @brief Create <Outputs> tag with necessary <Unknown>

   Our implementation puts all outputs before inputs, so inferring indices are
   straightforward, namely one-indexed with the number of outputs, starting with 1.

   @param [in] num_outputs Number of outputs
*/
std::string model_structure_outputs_generator(const std::uint32_t& num_outputs);
