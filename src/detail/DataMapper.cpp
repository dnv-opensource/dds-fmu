/*
  Copyright 2023, SINTEF Ocean
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "DataMapper.hpp"

#include <functional>
#include <vector>

#include <rapidxml/rapidxml.hpp>

#include "SignalDistributor.hpp" // resolve_type
#include "model-descriptor.hpp"

namespace ddsfmu {

void DataMapper::clear() {
  m_int_writer.clear();
  m_int_reader.clear();
  m_real_writer.clear();
  m_real_reader.clear();
  m_bool_writer.clear();
  m_bool_reader.clear();
  m_string_writer.clear();
  m_string_reader.clear();
  m_data_store.clear();
  m_int_offset = 0;
  m_real_offset = 0;
  m_bool_offset = 0;
  m_string_offset = 0;
}

void DataMapper::reset(const std::filesystem::path& fmu_resources) {
  clear();

  m_context = ddsfmu::config::load_fmu_idls(fmu_resources);

  auto ddsfmu_mapping = fmu_resources / "config" / "dds" / "ddsfmu_mapping.xml";
  std::vector<char> buffer;
  rapidxml::xml_document<> signal_mapping;
  ddsfmu::config::load_ddsfmu_mapping(signal_mapping, ddsfmu_mapping, buffer);

  auto mapper_ddsfmu = signal_mapping.first_node("ddsfmu");

  if (mapper_ddsfmu == nullptr) {
    throw std::runtime_error("<ddsfmu> not found in ddsfmu_mapping.xml");
  }

  auto mapper_iterator = [&](DataMapper::Direction direction) {
    std::string node_name;
    switch (direction) {
    case DataMapper::Direction::Write:
      // fmu input aka writer aka Setter aka DDS Publisher
      node_name = "fmu_in";
      break;
    case DataMapper::Direction::Read:
      // fmu output aka reader aka Getter aka DDS Subscriber
      node_name = "fmu_out";
      break;
    default: throw std::logic_error("DataMapper direction must be Read or Write");
    }

    for (rapidxml::xml_node<>* fmu_node = mapper_ddsfmu->first_node(node_name.c_str()); fmu_node;
         fmu_node = fmu_node->next_sibling(node_name.c_str())) {
      auto topic = fmu_node->first_attribute("topic");
      auto type = fmu_node->first_attribute("type");
      if (!topic || !type) {
        std::cerr << "<ddsfmu><" << node_name
                  << "> must specify attributes 'topic' and 'type'. Got: 'topic': "
                  << std::boolalpha << (topic != nullptr) << " and 'type': " << std::boolalpha
                  << (type != nullptr) << std::endl;
        throw std::runtime_error("Incomplete user data");
      }
      std::string topic_name(topic->value());
      std::string topic_type(type->value());
      bool do_key_filtering = false;

      if (direction == DataMapper::Direction::Read) {
        auto key_filter = fmu_node->first_attribute("key_filter");
        if (key_filter) {
          std::istringstream(key_filter->value()) >> std::boolalpha >> do_key_filtering;
        }
      }

      if (!m_context.module().has_structure(topic_type)) {
        std::cerr << "Got non-existing 'type': " << topic_type << std::endl;
        throw std::runtime_error("Unknown idl type");
      }

      add(topic_name, topic_type, direction);
      if (direction == DataMapper::Direction::Read && do_key_filtering) {
        queue_for_key_parameter(topic_name, topic_type);
      }
    }
  };

  mapper_iterator(DataMapper::Direction::Read);  // outputs
  mapper_iterator(DataMapper::Direction::Write); // inputs

  process_key_queue(); // parameters
}

void DataMapper::process_key_queue() {
  for (; !m_potential_keys.empty(); m_potential_keys.pop()) {
    const auto& couple = m_potential_keys.front();
    add(couple.first, couple.second, DataMapper::Direction::Parameter);
  }
}

void DataMapper::add(
  const std::string& topic_name, const std::string& topic_type, Direction read_write_param) {
  const eprosima::xtypes::DynamicType& message_type(m_context.module().structure(topic_type));
  DataMapper::StoreKey key = std::make_tuple(topic_name, read_write_param);

  auto item = m_data_store.emplace(key, eprosima::xtypes::DynamicData(message_type));

  if (!item.second) {
    std::string dir;
    switch (read_write_param) {
    case DataMapper::Direction::Write: dir = "input"; break;
    case DataMapper::Direction::Read: dir = "output"; break;
    case DataMapper::Direction::Parameter: dir = "parameter"; break;
    default: break;
    }

    throw std::runtime_error(
      std::string("Tried to create existing topic: ") + topic_name + std::string(" for FMU ")
      + dir);
  }

  auto& dyn_data = (*item.first).second;

  // We use reader indexes, they are identical to writers in this fmu
  DataMapper::IndexOffsets idx_value = std::make_tuple(
    m_real_reader.size(), m_int_reader.size(), m_bool_reader.size(), m_string_reader.size());
  m_offsets.emplace(key, idx_value);

  // switch on type kind must be identical to the one in SignalDistributor

  // FMU output
  // for each output: register dynamicdata store for requested type in a map (key: topic and Direction)
  //   for each type (primitive/string): register reader_visitor (read from dds into fmu, i.e. Get{Real,Integer..})
  //     valueRef is index of registered visitor for that type
  //     Note: we also register reader_visitor for FMU inputs (they are initial=exact by default).

  // FMU input
  // for each input: register dynamicdata store for requested type in a map (key: topic and Direction)
  //   for each type (primitive/string): register writer_visitor (write from fmu into dds, i.e. Set{Real,Integer..})
  //     valueRef is index of registered visitor for that type
  //     Note: we also register reader_visitor for FMU outputs, since they have initial=exact

  // FMU parameters
  // for each output: register dynamicdata store for requested type in a map (key: topic and Direction)
  //   for each type (primitive/string): register visitors if element is key
  //     valueRef is index of registered visitor for that type
  //     We register both reader and writer visitor for FMU parameters, initial=exact


  dyn_data.for_each([&](eprosima::xtypes::DynamicData::WritableNode& node) {
    bool is_leaf = (node.type().is_primitive_type() || node.type().is_enumerated_type());
    bool is_string = node.type().kind() == eprosima::xtypes::TypeKind::STRING_TYPE;
    bool is_leaf_or_string = is_leaf || is_string;
    bool is_not_parameter = read_write_param != DataMapper::Direction::Parameter;
    bool is_a_key_parameter = is_leaf_or_string && !is_not_parameter
                              && (node.from_member() && node.from_member()->is_key());

    if ((is_leaf_or_string && is_not_parameter) || is_a_key_parameter) {
      auto fmi_type = SignalDistributor::resolve_type(node);

      switch (fmi_type) {
      case ddsfmu::config::ScalarVariableType::Real:
        switch (node.type().kind()) {
        case eprosima::xtypes::TypeKind::FLOAT_32_TYPE:
          m_real_writer.emplace_back(
            std::bind(detail::writer_visitor<double, float>, std::placeholders::_1, node.data()));
          m_real_reader.emplace_back(
            std::bind(detail::reader_visitor<double, float>, std::placeholders::_1, node.data()));
          break;
        case eprosima::xtypes::TypeKind::FLOAT_64_TYPE:
          m_real_writer.emplace_back(
            std::bind(detail::writer_visitor<double, double>, std::placeholders::_1, node.data()));
          m_real_reader.emplace_back(
            std::bind(detail::reader_visitor<double, double>, std::placeholders::_1, node.data()));
          break;
        case eprosima::xtypes::TypeKind::UINT_32_TYPE:
          m_real_writer.emplace_back(std::bind(
            detail::writer_visitor<double, std::uint32_t>, std::placeholders::_1, node.data()));
          m_real_reader.emplace_back(std::bind(
            detail::reader_visitor<double, std::uint32_t>, std::placeholders::_1, node.data()));
          break;
        case eprosima::xtypes::TypeKind::INT_64_TYPE:
          m_real_writer.emplace_back(std::bind(
            detail::writer_visitor<double, std::int64_t>, std::placeholders::_1, node.data()));
          m_real_reader.emplace_back(std::bind(
            detail::reader_visitor<double, std::int64_t>, std::placeholders::_1, node.data()));
          break;
        case eprosima::xtypes::TypeKind::UINT_64_TYPE:
          m_real_writer.emplace_back(std::bind(
            detail::writer_visitor<double, std::uint64_t>, std::placeholders::_1, node.data()));
          m_real_reader.emplace_back(std::bind(
            detail::reader_visitor<double, std::uint64_t>, std::placeholders::_1, node.data()));
          break;
        default: break;
        }
        break;
      case ddsfmu::config::ScalarVariableType::Integer:
        switch (node.type().kind()) {
        case eprosima::xtypes::TypeKind::INT_8_TYPE:
          m_int_writer.emplace_back(std::bind(
            detail::writer_visitor<std::int32_t, std::int8_t>, std::placeholders::_1, node.data()));
          m_int_reader.emplace_back(std::bind(
            detail::reader_visitor<std::int32_t, std::int8_t>, std::placeholders::_1, node.data()));
          break;
        case eprosima::xtypes::TypeKind::UINT_8_TYPE:
          m_int_writer.emplace_back(std::bind(
            detail::writer_visitor<std::int32_t, std::uint8_t>, std::placeholders::_1,
            node.data()));
          m_int_reader.emplace_back(std::bind(
            detail::reader_visitor<std::int32_t, std::uint8_t>, std::placeholders::_1,
            node.data()));
          break;
        case eprosima::xtypes::TypeKind::INT_16_TYPE:
          m_int_writer.emplace_back(std::bind(
            detail::writer_visitor<std::int32_t, std::int16_t>, std::placeholders::_1,
            node.data()));
          m_int_reader.emplace_back(std::bind(
            detail::reader_visitor<std::int32_t, std::int16_t>, std::placeholders::_1,
            node.data()));
          break;
        case eprosima::xtypes::TypeKind::UINT_16_TYPE:
          m_int_writer.emplace_back(std::bind(
            detail::writer_visitor<std::int32_t, std::uint16_t>, std::placeholders::_1,
            node.data()));
          m_int_reader.emplace_back(std::bind(
            detail::reader_visitor<std::int32_t, std::uint16_t>, std::placeholders::_1,
            node.data()));
          break;
        case eprosima::xtypes::TypeKind::INT_32_TYPE:
          m_int_writer.emplace_back(std::bind(
            detail::writer_visitor<std::int32_t, std::int32_t>, std::placeholders::_1,
            node.data()));
          m_int_reader.emplace_back(std::bind(
            detail::reader_visitor<std::int32_t, std::int32_t>, std::placeholders::_1,
            node.data()));
          break;
        case eprosima::xtypes::TypeKind::ENUMERATION_TYPE:
          m_int_writer.emplace_back(std::bind(
            detail::writer_visitor<std::int32_t, std::uint32_t>, std::placeholders::_1,
            node.data()));
          m_int_reader.emplace_back(std::bind(
            detail::reader_visitor<std::int32_t, std::uint32_t>, std::placeholders::_1,
            node.data()));
          break;
        default: break;
        }
        break;
      case ddsfmu::config::ScalarVariableType::Boolean:
        switch (node.type().kind()) {
        case eprosima::xtypes::TypeKind::BOOLEAN_TYPE:
          m_bool_writer.emplace_back(
            std::bind(detail::writer_visitor<bool, bool>, std::placeholders::_1, node.data()));
          m_bool_reader.emplace_back(
            std::bind(detail::reader_visitor<bool, bool>, std::placeholders::_1, node.data()));
          break;
        default: break;
        }
        break;
      case ddsfmu::config::ScalarVariableType::String:
        switch (node.type().kind()) {
        case eprosima::xtypes::TypeKind::STRING_TYPE:
          m_string_writer.emplace_back(std::bind(
            detail::writer_visitor<std::string, std::string>, std::placeholders::_1, node.data()));
          m_string_reader.emplace_back(std::bind(
            detail::reader_visitor<std::string, std::string>, std::placeholders::_1, node.data()));
          break;
        case eprosima::xtypes::TypeKind::CHAR_8_TYPE:
          m_string_writer.emplace_back(std::bind(
            detail::writer_visitor<std::string, char>, std::placeholders::_1, node.data()));
          m_string_reader.emplace_back(std::bind(
            detail::reader_visitor<std::string, char>, std::placeholders::_1, node.data()));
          break;
        default: break;
        }
        break;
      default: break;
      }
    }
  });
}

}
