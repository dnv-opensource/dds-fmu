/*
  Copyright 2023, SINTEF Ocean
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "SignalDistributor.hpp"

namespace ddsfmu {

SignalDistributor::SignalDistributor()
    : m_real_idx(0), m_integer_idx(0), m_boolean_idx(0), m_string_idx(0), m_outputs(0) {}

void SignalDistributor::load_idls(const std::filesystem::path& resource_path) {
  m_context = ddsfmu::config::load_fmu_idls(resource_path);
}

bool SignalDistributor::has_structure(const std::string& topic_type) {
  return m_context.module().has_structure(topic_type);
}

void SignalDistributor::add(
  const std::string& topic_name, const std::string& topic_type, Cardinality cardinal) {
  const eprosima::xtypes::DynamicType& message_type(m_context.module().structure(topic_type));
  eprosima::xtypes::DynamicData message_data(message_type);

  std::string cardinal_string, cardinality_prefix;
  switch (cardinal) {
  case Cardinality::INPUT:
    cardinal_string = "input";
    cardinality_prefix = "pub.";
    break;
  case Cardinality::OUTPUT:
    cardinal_string = "output";
    cardinality_prefix = "sub.";
    break;
  case Cardinality::PARAMETER:
    cardinal_string = "parameter";
    cardinality_prefix = "key.sub.";
    break;
  }

  message_data.for_each([&](eprosima::xtypes::DynamicData::ReadableNode& node) {
    bool is_leaf = (node.type().is_primitive_type() || node.type().is_enumerated_type());
    bool is_string = node.type().kind() == eprosima::xtypes::TypeKind::STRING_TYPE;
    bool is_leaf_or_string = is_leaf || is_string;
    bool is_not_parameter = cardinal != Cardinality::PARAMETER;
    bool is_a_key_parameter = is_leaf_or_string && !is_not_parameter
     && (node.from_member() && node.from_member()->is_key());

    if ((is_leaf_or_string && is_not_parameter) || is_a_key_parameter) {
      if (cardinal == Cardinality::OUTPUT) { m_outputs++; }

      std::string structured_name;
      config::name_generator(structured_name, node);
      structured_name = cardinality_prefix + topic_name + "." + structured_name;

      auto fmi_type = SignalDistributor::resolve_type(node);

      switch (fmi_type) {
      case config::ScalarVariableType::Real:
        m_signal_mapping.emplace_back(
          std::make_tuple(m_real_idx++, structured_name, cardinal_string, fmi_type));
        break;
      case config::ScalarVariableType::Integer:
        m_signal_mapping.emplace_back(
          std::make_tuple(m_integer_idx++, structured_name, cardinal_string, fmi_type));
        break;
      case config::ScalarVariableType::Boolean:
        m_signal_mapping.emplace_back(
          std::make_tuple(m_boolean_idx++, structured_name, cardinal_string, fmi_type));
        break;
      case config::ScalarVariableType::String:
        m_signal_mapping.emplace_back(
          std::make_tuple(m_string_idx++, structured_name, cardinal_string, fmi_type));
        break;
      default: break;
      }
    }
  });
}

void SignalDistributor::process_key_queue() {
  for (; !m_potential_keys.empty(); m_potential_keys.pop()) {
    const auto& couple = m_potential_keys.front();
    add(couple.first, couple.second, Cardinality::PARAMETER);
  }
}


ddsfmu::config::ScalarVariableType
  SignalDistributor::resolve_type(const eprosima::xtypes::DynamicData::ReadableNode& node) {
  switch (node.type().kind()) {
  case eprosima::xtypes::TypeKind::BOOLEAN_TYPE: return ddsfmu::config::ScalarVariableType::Boolean;
  case eprosima::xtypes::TypeKind::INT_8_TYPE:
  case eprosima::xtypes::TypeKind::UINT_8_TYPE:
  case eprosima::xtypes::TypeKind::INT_16_TYPE:
  case eprosima::xtypes::TypeKind::UINT_16_TYPE:
  case eprosima::xtypes::TypeKind::INT_32_TYPE: return ddsfmu::config::ScalarVariableType::Integer;
  case eprosima::xtypes::TypeKind::FLOAT_32_TYPE:
  case eprosima::xtypes::TypeKind::FLOAT_64_TYPE: return ddsfmu::config::ScalarVariableType::Real;

  case eprosima::xtypes::TypeKind::STRING_TYPE:
  case eprosima::xtypes::TypeKind::CHAR_8_TYPE: return ddsfmu::config::ScalarVariableType::String;
  case eprosima::xtypes::TypeKind::ENUMERATION_TYPE:
    return ddsfmu::config::ScalarVariableType::Integer;
  case eprosima::xtypes::TypeKind::UINT_32_TYPE:
  case eprosima::xtypes::TypeKind::INT_64_TYPE:
  case eprosima::xtypes::TypeKind::UINT_64_TYPE: return ddsfmu::config::ScalarVariableType::Real;
  case eprosima::xtypes::TypeKind::FLOAT_128_TYPE:
  case eprosima::xtypes::TypeKind::CHAR_16_TYPE:
  case eprosima::xtypes::TypeKind::WIDE_CHAR_TYPE:
  case eprosima::xtypes::TypeKind::BITSET_TYPE:
  case eprosima::xtypes::TypeKind::ALIAS_TYPE:    // Needed?
  case eprosima::xtypes::TypeKind::SEQUENCE_TYPE: // std::vector
  case eprosima::xtypes::TypeKind::WSTRING_TYPE:
  case eprosima::xtypes::TypeKind::MAP_TYPE:
  default:
    std::cerr << "Unsupported type: " << node.type().name() << std::endl;
    return config::ScalarVariableType::Unknown;
  }
}

}
