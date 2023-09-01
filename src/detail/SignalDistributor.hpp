#pragma once

#include <cstdint>
#include <filesystem>
#include <tuple>
#include <vector>

#include <xtypes/idl/idl.hpp>

#include "auxiliaries.hpp"
#include "model-descriptor.hpp"

namespace ddsfmu {

typedef std::tuple<std::uint32_t, std::string, std::string, config::ScalarVariableType> SignalInfo;

/**
   @brief Signal distributor helper for creating model description

   It loads both the IDL files and ddsfmu mapping configuration to establish correct
   entries in the modelDescription.xml. This entails defining entries in
   `<ModelVariables>` as `<ScalarVariable>` with structured name, value references and
   causality. Moreover, it provides the number of outputs, so that `<ModelStructure>`
   `<Outputs>` can be generated.

*/
class SignalDistributor {
public:
  SignalDistributor(); ///< Constructor that sets integer members to 0;
  /**
     @brief Resolves the FMI primitive type given an xtypes::DynamicData node

     Internally, a switch case of xtypes::TypeKind types that maps to a ScalarVariableType
     In case the type is unsupported: returns ddsfmu::config::Unknown;

     @param [in] node A node, which is a primitive type or resolvable type (std::string and maybe other)
     @return The scalar variable type (Real, Integer, Boolean, String)

  */
  static config::ScalarVariableType
    resolve_type(const eprosima::xtypes::DynamicData::ReadableNode& node);

  /**
     @brief Loads the IDLs into a member variable

     @param [in] resource_path Path to the FMU resources folder
  */
  void load_idls(const std::filesystem::path& resource_path);

  /**
     @brief Returns true if IDL has the scoped structure topic_type

     This function simply passes the query further on to xtypes::idl::Context::module().

     @param [in] topic_type Scoped name of the queried type (e.g. My::Impl)
     @return The result of the check
  */
  bool has_structure(const std::string& topic_type);

  /**
     @brief  Adds signal mappings in form of SignalInfo entries

     This function resolves the FMU types for each member of a specified topic type.  The
     structured name used in the ScalarVariable is `[topic].[structured_name]`, where
     structured_name is defined according to FMU structured name convention.

     @param [in] topic_name Topic name
     @param [in] topic_type Type name
     @param [in] is_in_not_out Whether the signal is input and not output
  */
  void add(const std::string& topic_name, const std::string& topic_type, bool is_in_not_out);
  inline const std::vector<SignalInfo>& get_mapping() const {
    return m_signal_mapping;
  } ///< Returns reference to the vector of SignalInfo.
  inline std::uint32_t outputs() const {
    return m_outputs;
  } ///< Returns number of scalar FMU outputs
private:
  std::uint32_t m_real_idx, m_integer_idx, m_boolean_idx, m_string_idx, m_outputs;
  std::vector<SignalInfo> m_signal_mapping;
  eprosima::xtypes::idl::Context m_context;
};

}
