/*
  Copyright 2023, SINTEF Ocean
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "DynamicPubSub.hpp"

#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include <cppfmu_common.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicPubSubType.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypePtr.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include "Converter.hpp"
#include "LoggerAdapters.hpp"
#include "model-descriptor.hpp"

namespace ddsfmu {

DynamicPubSub::DynamicPubSub()
    : m_participant(nullptr)
    , m_subscriber(nullptr)
    , m_publisher(nullptr)
    , m_data_mapper(nullptr)
    , m_xml_loaded(false) {}

void DynamicPubSub::write() {
  for (auto& writes : m_write_data) {
    ddsfmu::Converter::xtypes_to_fastdds(writes.second.first, writes.second.second.get());
    writes.first->write(static_cast<void*>(writes.second.second.get()));
  }
}

DynamicPubSub::~DynamicPubSub() { clear(); }

void DynamicPubSub::init_key_filters() {
  // Once DataReader has been created, update filter with Reader GUID
  // This call should also be done once we know that initialization
  // has updated parameter values with writer visitors

  for (auto& [reader, filter] : m_reader_topic_filter) {
    std::stringstream guid;
    guid << reader->guid();
    std::vector<std::string> new_params;
    new_params.emplace_back(guid.str()); // Reader GUID

    auto parameter_data = m_filter_data.at(filter);

    // Acquire and convert from DynamicData into string
    parameter_data.for_each([&](const eprosima::xtypes::DynamicData::ReadableNode& node) {
      bool is_leaf = (node.type().is_primitive_type() || node.type().is_enumerated_type());
      bool is_string = node.type().kind() == eprosima::xtypes::TypeKind::STRING_TYPE;
      if ((is_leaf || is_string) && node.from_member() && node.from_member()->is_key()) {
        switch (node.type().kind()) {
        case eprosima::xtypes::TypeKind::BOOLEAN_TYPE: {
          std::ostringstream oss;
          oss << std::boolalpha << node.data().value<bool>();
          new_params.emplace_back(oss.str());
          break;
        }
        case eprosima::xtypes::TypeKind::INT_8_TYPE:
          new_params.emplace_back(std::to_string(node.data().value<std::int8_t>()));
          break;
        case eprosima::xtypes::TypeKind::UINT_8_TYPE:
          new_params.emplace_back(std::to_string(node.data().value<std::uint8_t>()));
          break;
        case eprosima::xtypes::TypeKind::INT_16_TYPE:
          new_params.emplace_back(std::to_string(node.data().value<std::int16_t>()));
          break;
        case eprosima::xtypes::TypeKind::UINT_16_TYPE:
          new_params.emplace_back(std::to_string(node.data().value<std::uint16_t>()));
          break;
        case eprosima::xtypes::TypeKind::INT_32_TYPE:
          new_params.emplace_back(std::to_string(node.data().value<std::int32_t>()));
          break;
        case eprosima::xtypes::TypeKind::FLOAT_32_TYPE:
          new_params.emplace_back(std::to_string(node.data().value<float>()));
          break;
        case eprosima::xtypes::TypeKind::FLOAT_64_TYPE:
          new_params.emplace_back(std::to_string(node.data().value<double>()));
          break;
        case eprosima::xtypes::TypeKind::STRING_TYPE:
          new_params.emplace_back(node.data().value<std::string>());
          break;
        case eprosima::xtypes::TypeKind::CHAR_8_TYPE:
          new_params.emplace_back(std::string(1, node.data().value<char>()));
          break;
        case eprosima::xtypes::TypeKind::ENUMERATION_TYPE:
          new_params.emplace_back(std::to_string(node.data().value<std::uint32_t>()));
          break;
        case eprosima::xtypes::TypeKind::UINT_32_TYPE:
          new_params.emplace_back(std::to_string(node.data().value<std::uint32_t>()));
          break;
        case eprosima::xtypes::TypeKind::INT_64_TYPE:
          new_params.emplace_back(std::to_string(node.data().value<std::int64_t>()));
          break;
        case eprosima::xtypes::TypeKind::UINT_64_TYPE:
          new_params.emplace_back(std::to_string(node.data().value<std::uint64_t>()));
          break;
        case eprosima::xtypes::TypeKind::FLOAT_128_TYPE:
        case eprosima::xtypes::TypeKind::CHAR_16_TYPE:
        case eprosima::xtypes::TypeKind::WIDE_CHAR_TYPE:
        case eprosima::xtypes::TypeKind::BITSET_TYPE:
        case eprosima::xtypes::TypeKind::ALIAS_TYPE:
        case eprosima::xtypes::TypeKind::SEQUENCE_TYPE:
        case eprosima::xtypes::TypeKind::WSTRING_TYPE:
        case eprosima::xtypes::TypeKind::MAP_TYPE:
        default: throw std::runtime_error("Tried to set parameter of unsupported TypeKind");
        }
      }
    });

    filter->set_expression_parameters(new_params);
  }
}

void DynamicPubSub::take() {
  for (auto& reads : m_read_data) {
    auto have_data = eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK;
    eprosima::fastrtps::types::ReturnCode_t exec_result = have_data;
    eprosima::fastdds::dds::SampleInfo info;

    while (exec_result == have_data) {
      exec_result = reads.first->take_next_sample(reads.second.second.get(), &info);
      if (exec_result == have_data) {
        ddsfmu::Converter::fastdds_to_xtypes(reads.second.second.get(), reads.second.first);
      }
    }
  }
}

void DynamicPubSub::clear() {
  auto* participant_factory = eprosima::fastdds::dds::DomainParticipantFactory::get_instance();

  eprosima::fastdds::dds::Log::ClearConsumers(); // Clear both default stdcout and custom logger

  if (m_participant) { m_participant->set_listener(nullptr); }

  // Clean-up old instances, if they exist
  for (auto& item : m_write_data) {
    // Not needed when using DynamicData_ptr
    //eprosima::fastrtps::types::DynamicDataFactory::get_instance()->delete_data(item.second.second);

    item.first->set_listener(nullptr);
    m_publisher->delete_datawriter(item.first);
  }

  if (m_publisher) { m_participant->delete_publisher(m_publisher); }
  m_publisher = nullptr;

  for (auto& item : m_read_data) {
    // Not needed when using DynamicData_ptr
    //eprosima::fastrtps::types::DynamicDataFactory::get_instance()->delete_data(item.second.second);
    item.first->set_listener(nullptr);
    m_subscriber->delete_datareader(item.first);
  }
  if (m_subscriber) { m_participant->delete_subscriber(m_subscriber); }
  m_subscriber = nullptr;

  for (auto& item : m_topic_name_ptr) { m_participant->delete_topic(item.second); }

  for (auto& item : m_reader_topic_filter) {
    m_participant->delete_contentfilteredtopic(item.second);
  }

  if (m_participant) m_participant->delete_contained_entities(); // e.g filter factory

  auto operation_ok = eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK;

  ddsfmu::Converter::clear_data_structures();

  if (m_participant && operation_ok != participant_factory->delete_participant(m_participant)) {
    throw std::runtime_error("Could not successfully delete DDS domain participant");
  }

  m_topic_to_type.clear();
  m_types.clear();
  m_topic_name_ptr.clear();
  m_reader_topic_filter.clear();
  m_write_data.clear();
  m_read_data.clear();
}

void DynamicPubSub::reset(
  const std::filesystem::path& fmu_resources, DataMapper* const mapper_ptr, const std::string& name,
  cppfmu::Logger* const logger) {
  clear();
  m_data_mapper = mapper_ptr;

  // Load and create new instances

  if (logger) {
    // This adds a custom FMI logger to fast-dds
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Kind::Info);
    eprosima::fastdds::dds::Log::ReportFunctions(false);
    eprosima::fastdds::dds::Log::RegisterConsumer(
      std::make_unique<ddsfmu::FmiLogger>(*logger, name));
  }

  if (!m_xml_loaded) {
    // only load once
    auto dds_profile = fmu_resources / "config" / "dds" / "dds_profile.xml";

    if (
      eprosima::fastrtps::xmlparser::XMLP_ret::XML_OK
      != eprosima::fastrtps::xmlparser::XMLProfileManager::loadXMLFile(dds_profile.string())) {
      std::cerr << "Cannot load XML file " << dds_profile << std::endl;
      throw std::runtime_error("Unable to load DDS XML profile");
    }

    m_xml_loaded = true;
  }

  namespace edds = eprosima::fastdds::dds;
  namespace etypes = eprosima::fastrtps::types;

  // Note: We create only one participant for each fmu
  m_participant = edds::DomainParticipantFactory::get_instance()->create_participant_with_profile(
    "dds-fmu-default");

  if (!m_participant) { throw std::runtime_error("Could not create domain participant"); }

  m_publisher = m_participant->create_publisher_with_profile("dds-fmu-default");
  m_subscriber = m_participant->create_subscriber_with_profile("dds-fmu-default");

  if (!m_publisher) { throw std::runtime_error("Could not create publisher"); }
  if (!m_subscriber) { throw std::runtime_error("Could not create subscriber"); }

  if (
    eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK
    != m_participant->register_content_filter_factory("CUSTOM_KEY_FILTER", &m_filter_factory)) {
    throw std::runtime_error("Could not register custom key filter factory");
  }

  // load ddsfmu mapping
  rapidxml::xml_document<> doc;
  std::vector<char> buffer;

  ddsfmu::config::load_ddsfmu_mapping(
    doc, fmu_resources / "config" / "dds" / "ddsfmu_mapping.xml", buffer);

  auto root_node = doc.first_node("ddsfmu");

  typedef std::vector<std::tuple<std::string, std::string, DynamicPubSub::PubOrSub>> SignalList;
  SignalList fmu_signals;

  // This lambda loads topic and type from <fmu_in> and <fmu_out> of <ddsfmu> the ddsfmu mapping xml
  auto xml_loader = [&](const std::string& node_name, SignalList& signals) {
    DynamicPubSub::PubOrSub sig_type;
    if (node_name == "fmu_in") {
      sig_type = DynamicPubSub::PubOrSub::PUBLISH;
    } else {
      sig_type = DynamicPubSub::PubOrSub::SUBSCRIBE;
    }

    for (rapidxml::xml_node<>* fmu_node = root_node->first_node(node_name.c_str()); fmu_node;
         fmu_node = fmu_node->next_sibling(node_name.c_str())) {
      auto topic = fmu_node->first_attribute("topic");
      auto type = fmu_node->first_attribute("type");
      if (!topic || !type) {
        std::cerr << "<ddsfmu><" << node_name
                  << "> must specify attributes 'topic' and 'type' Got 'topic': " << std::boolalpha
                  << (topic != nullptr) << " and 'type': " << std::boolalpha << (type != nullptr)
                  << std::endl;
        throw std::runtime_error("Erroneous <ddsfmu>");
      }
      if (!mapper().idl_context().module().has_structure(std::string(type->value()))) {
        throw std::runtime_error("Requested unknown type: " + std::string(type->value()));
      }
      signals.emplace_back(std::make_tuple(topic->value(), type->value(), sig_type));
    }
  };

  xml_loader("fmu_in", fmu_signals);  // publishers
  xml_loader("fmu_out", fmu_signals); // subscribers


  /*
    For each topic name, type name and dds direction (read or write)
    1. Get xtypes DynamicType
    2. Retrieve DynamicTypeBuilder (fast-dds) for xtypes DynamicType
    3. Register type if not registered
    4. Create topic if not already created
    5. Create DynamicData_ptr for DDS to write/read to/from.
    6. Create DataWriter and DataReader using profile, otherwise use default profile
    7. Add data store with connection between DDS DynamicData_ptr and xtypes::DynamicData
      xtypes::DynamicData is a reference to data owned by DataMapper.

  */
  for (auto& topic_type : fmu_signals) {
    // Get xtypes DynamicType
    const eprosima::xtypes::DynamicType& message_type(
      mapper().idl_context().module().structure(std::get<1>(topic_type)));

    // Retrieve DynamicTypeBuilder for xtypes DynamicType
    etypes::DynamicTypeBuilder* builder = ddsfmu::Converter::create_builder(message_type);

    if (!builder) {
      throw std::runtime_error("Could not create builder for type: " + std::get<1>(topic_type));
    }

    bool skip_register = false;

    // Register dynamic type with participant by providing topic name and type name

    if (m_topic_to_type.find(std::get<0>(topic_type)) != m_topic_to_type.end()) {
      skip_register = true;
    }

    if (!skip_register && m_types.find(std::get<1>(topic_type)) != m_types.end()) {
      m_topic_to_type.emplace(std::get<0>(topic_type), std::get<1>(topic_type));
      skip_register = true; // type registered
    }

    if (!skip_register) {
      etypes::DynamicType_ptr dyntype_ptr = builder->build();

      if (!dyntype_ptr) {
        throw std::runtime_error(
          "Could not create fastrtps dynamic type ptr for: " + std::get<0>(topic_type)
          + " of type: " + std::get<1>(topic_type));
      }

      auto reg_type =
        m_types.emplace(std::get<1>(topic_type), etypes::DynamicPubSubType(dyntype_ptr));

      bool added = reg_type.second;
      etypes::DynamicPubSubType& dyn_type_support = reg_type.first->second;
      m_topic_to_type.emplace(std::get<0>(topic_type), std::get<1>(topic_type));

      edds::TypeSupport p_type = m_participant->find_type(std::get<1>(topic_type));

      // Check if already registered with dds participant
      if (!p_type) { // not registered
        dyn_type_support.setName(std::get<1>(topic_type).c_str());
        // A bug with UnionType in Fast DDS Dynamic Types is bypassed.
        // WORKAROUND START
        dyn_type_support.auto_fill_type_information(false); // True will not work with CycloneDDS
        dyn_type_support.auto_fill_type_object(
          false); // True causes seg fault with enums and other complex types, etc sequences of structs
        // WORKAROUND END

        m_participant->register_type(dyn_type_support);
      }

      if (added) {
        // Is this really needed?
        ddsfmu::Converter::register_type(std::get<1>(topic_type), &dyn_type_support);
        ddsfmu::Converter::register_xtype(std::get<1>(topic_type), message_type);
      }
    }

    auto topic_description = m_participant->lookup_topicdescription(std::get<0>(topic_type));

    edds::Topic* tmp_topic = nullptr;

    if (!topic_description) {
      tmp_topic = m_participant->create_topic_with_profile(
        std::get<0>(topic_type), std::get<1>(topic_type), std::get<0>(topic_type));
      if (!tmp_topic) {
        // TODO: add log entry about using default topic qos
        tmp_topic = m_participant->create_topic(
          std::get<0>(topic_type), std::get<1>(topic_type), edds::TOPIC_QOS_DEFAULT);
      }
      if (!tmp_topic) {
        throw std::runtime_error(
          "Unable to create topic: " + std::get<0>(topic_type) + " of type "
          + std::get<1>(topic_type));
      }

    } else {
      tmp_topic = static_cast<edds::Topic*>(topic_description);
    }
    m_topic_name_ptr.emplace(std::get<0>(topic_type), tmp_topic);

    const etypes::DynamicType_ptr& dynamic_type =
      m_types.at(m_topic_to_type.at(std::get<0>(topic_type))).GetDynamicType();

    etypes::DynamicData* dynamic_data_ptr =
      etypes::DynamicDataFactory::get_instance()->create_data(dynamic_type);

    if (std::get<2>(topic_type) == PubOrSub::PUBLISH) {
      edds::DataWriter* tmp_writer =
        m_publisher->create_datawriter_with_profile(tmp_topic, std::get<0>(topic_type));

      if (!tmp_writer) {
        // TODO: add log entry about using default datawriter qos
        tmp_writer = m_publisher->create_datawriter(tmp_topic, edds::DATAWRITER_QOS_DEFAULT);
      }
      if (!tmp_writer) {
        throw std::runtime_error(
          "Unable to create DataWriter for topic: " + std::get<1>(topic_type));
      }

      m_write_data.emplace(std::make_pair(
        tmp_writer,
        std::make_pair(
          std::ref(mapper().data_ref(std::get<0>(topic_type), DataMapper::Direction::Write)),
          dynamic_data_ptr)));
    } else {
      bool need_filter = false;

      try {
        // If user has requested key_filter=True, it is registered in DataMapper
        auto parameter_data =
          mapper().data_ref(std::get<0>(topic_type), DataMapper::Direction::Parameter);

        // Iterate members to see if at least one member is key
        parameter_data.for_each([&](const eprosima::xtypes::DynamicData::ReadableNode& a_node) {
          bool a_is_leaf =
            (a_node.type().is_primitive_type() || a_node.type().is_enumerated_type());
          bool a_is_string = a_node.type().kind() == eprosima::xtypes::TypeKind::STRING_TYPE;
          if (
            (a_is_leaf || a_is_string) && a_node.from_member() && a_node.from_member()->is_key()) {
            need_filter = true;
            throw false; // Found at least one key, so break for_each
          }
        });
      } catch (const std::out_of_range& no_key) {
        /* Not registered in DataMapper, no key filtering */
      }

      eprosima::fastdds::dds::ContentFilteredTopic* filter_topic = nullptr;

      if (need_filter) {
        filter_topic = m_participant->create_contentfilteredtopic(
          std::get<0>(topic_type) + "Filtered", tmp_topic, " ", {"|GUID UNKNOWN|", "0"},
          "CUSTOM_KEY_FILTER");

        if (filter_topic == nullptr) {
          throw std::runtime_error(
            "Unable to create filtered topic for: " + std::get<1>(topic_type));
        }
      }

      edds::DataReader* tmp_reader = nullptr;

      if (!need_filter) {
        m_subscriber->create_datareader_with_profile(tmp_topic, std::get<0>(topic_type));
      } else {
        m_subscriber->create_datareader_with_profile(filter_topic, std::get<0>(topic_type));
      }

      if (!tmp_reader) {
        // TODO: add log entry about using default datareader qos
        if (!need_filter) {
          tmp_reader = m_subscriber->create_datareader(tmp_topic, edds::DATAREADER_QOS_DEFAULT);
        } else {
          tmp_reader = m_subscriber->create_datareader(filter_topic, edds::DATAREADER_QOS_DEFAULT);
        }
      }
      if (!tmp_reader) {
        throw std::runtime_error(
          "Unable to create DataReader for topic: " + std::get<1>(topic_type));
      }

      if (need_filter) {
        m_reader_topic_filter.emplace(tmp_reader, filter_topic);
        m_filter_data.emplace(
          filter_topic,
          std::ref(mapper().data_ref(std::get<0>(topic_type), DataMapper::Direction::Parameter)));
      }

      m_read_data.emplace(std::make_pair(
        tmp_reader,
        std::make_pair(
          std::ref(mapper().data_ref(std::get<0>(topic_type), DataMapper::Direction::Read)),
          dynamic_data_ptr)));
    }
  }
}

}
