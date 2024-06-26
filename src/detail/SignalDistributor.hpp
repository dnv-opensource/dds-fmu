#pragma once

/*
  Copyright 2023, SINTEF Ocean
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <cstdint>
#include <filesystem>
#include <queue>
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

  enum class Cardinality { INPUT, OUTPUT, PARAMETER };

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
     structured name used in the ScalarVariable is `[pubsub].[topic].[structured_name]`, where
     structured_name is defined according to FMU structured name convention, and`pubsub`
     is `pub` or `sub`, depending on whether it is a published or subscribed signal.

     @param [in] topic_name Topic name
     @param [in] topic_type Type name
     @param [in] cardinal Whether the signal is input, output, or parameter
  */
  void add(const std::string& topic_name, const std::string& topic_type, Cardinality cardinal);

  /**
     @brief  Queues signal mappings in form of SignalInfo entries for key parameters, if any

     This function resolves the FMU types for each member of a specified topic type.  The
     structured name used in the ScalarVariable is `key.sub.[topic].[structured_name]`,
     where structured_name is defined according to FMU structured name convention,
     and 'key' indicate that it is a key parameter to be selected.

     The user must run process_key_queue() after this function has been run for all
     outputs.

     @param [in] topic_name Topic name
     @param [in] topic_type Type name
  */
  inline void queue_for_key_parameter(const std::string& topic_name, const std::string& topic_type){
    m_potential_keys.push(std::make_pair(topic_name, topic_type));
  }

  /**
     @brief  Processes queued output data by adding them to the signal mapping

     This function creates SignalInfo and appends them to the vector, which can be
     retrieved by get_mapping(). Parameters are added after outputs and inputs.
  */
  void process_key_queue();

  inline const std::vector<SignalInfo>& get_mapping() const {
    return m_signal_mapping;
  } ///< Returns reference to the vector of SignalInfo.
  inline std::uint32_t outputs() const {
    return m_outputs;
  } ///< Returns number of scalar FMU outputs
private:
  std::queue<std::pair<std::string, std::string>> m_potential_keys;
  std::uint32_t m_real_idx, m_integer_idx, m_boolean_idx, m_string_idx, m_outputs;
  std::vector<SignalInfo> m_signal_mapping;
  eprosima::xtypes::idl::Context m_context;
};

}
