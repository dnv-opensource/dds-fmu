#pragma once

/*
  Copyright 2023, SINTEF Ocean
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <filesystem>
#include <functional>
#include <map>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastrtps/types/DynamicPubSubType.h>

#include "CustomKeyFilterFactory.hpp"
#include "DataMapper.hpp"

namespace cppfmu {
class Logger;
}

namespace ddsfmu {

class DomainListener : public eprosima::fastdds::dds::DomainParticipantListener {
  static const std::map<uint32_t /*eprosima::fastdds::dds::QosPolicyId_t*/, std::string>
    QosPolicyString;

public:
  DomainListener() = default;
  ~DomainListener() override = default;

  void on_requested_incompatible_qos(
    eprosima::fastdds::dds::DataReader* reader,
    const eprosima::fastdds::dds::RequestedIncompatibleQosStatus& status) override {
    std::ostringstream oss;
    oss << "The data reader " << reader->guid() << " with topic name '"
        << reader->get_topicdescription()->get_name() << "' of type '"
        << reader->get_topicdescription()->get_type_name()
        << "' has requested incompatible QoS with the one offered by the writer:";
    for (const auto& policy : status.policies) {
      if (policy.count > 0) {
        try {
          oss << " " << QosPolicyString.at(policy.policy_id) << ", ";
        } catch (const std::out_of_range& e) { oss << "[Unknown policy!]" << std::endl; }
      }
    }
    oss << std::endl;
    //std::cout << oss.str();
  }

  void on_offered_incompatible_qos(
    eprosima::fastdds::dds::DataWriter* writer,
    const eprosima::fastdds::dds::OfferedIncompatibleQosStatus& status) override {
    std::ostringstream oss;
    oss << "The data writer " << writer->guid() << " with topic name '"
        << writer->get_topic()->get_name() << "' of type '" << writer->get_topic()->get_type_name()
        << "' has offered incompatible QoS with the one requested by the reader:";
    for (const auto& policy : status.policies) {
      if (policy.count > 0) {
        try {
          oss << " " << QosPolicyString.at(policy.policy_id) << ", ";
        } catch (const std::out_of_range& e) { oss << "[Unknown policy!]" << std::endl; }
      }
    }
    oss << std::endl;
    //std::cout << oss.str();
  }

  void on_inconsistent_topic(
    eprosima::fastdds::dds::Topic* topic,
    eprosima::fastdds::dds::InconsistentTopicStatus status) override {
    // TODO: Not yet implemented by fast-dds and will never trigger (fast-dds v2.11.2)
    std::ostringstream oss;
    oss << "There already exist another topic with inconsistent characteristics for topic name '"
        << topic->get_name() << "' of type '" << topic->get_type_name() << "'" << std::endl;
    //std::cout << oss.str();
  }
};

/**
   @brief Dynamic Publisher and Subscriber

   This class consists of one each instances: A DDS Domain Participant, a dds::Publisher,
   and a dds::Subscriber. It loads a Fast-DDS XML profile, uses IDL files for type
   specification, and a DDS to FMU mapping configuration file.  For each mapping of DDS
   topic, it registers either a DataWriter or DataReader entity.  It has convenience
   functions to call write or take on all registered entities.  By means of a converter,
   the inbound or outbound DDS data are populated in a connected DataMapper instance.

*/
class DynamicPubSub {
public:
  DynamicPubSub();  ///< Default constructor sets pointers to nullptr and m_xml_load ed false
  ~DynamicPubSub(); ///< Destructor calls clear()
  DynamicPubSub(const DynamicPubSub&) = delete;            ///< Copy constructor
  DynamicPubSub& operator=(const DynamicPubSub&) = delete; ///< Copy assignment

  /**
     @brief Resets all members of DynamicPubSub

     Calls clear(), then loads configuration files and initializes DDS members, as well as other data structures.

     @param [in] fmu_resources Path to FMU resources folder
     @param [in] mapper Pointer to DataMapper instance to be used
     @param [in] name Instance name of FMU
     @param [in] logger Pointer to FMI logger dispatcher

  */
  void reset(
    const std::filesystem::path& fmu_resources, DataMapper* const mapper,
    const std::string& name = "dds-fmu", cppfmu::Logger* const logger = nullptr);

  /**
     @brief Writes DDS data by using data from DataMapper

     For each DataWriter: Converts associated xtypes::DynamicData to DynamicData_ptr and publishes it
  */
  void write();

  /**
     @brief Takes DDS data into data in DataMapper

     For each DataReader: Takes data from DDS and if data: Converts to associated xtypes::DynamicData
  */
  void take();

  /**
     @brief Initialize content filters for keyed topics

     For each ContentFilteredTopic: Update filter parameters with reader GUID and key
     valuesfor which filtering will occur
  */
  void init_key_filters();

private:
  typedef std::pair<eprosima::xtypes::DynamicData&, eprosima::fastrtps::types::DynamicData_ptr>
    DynamicDataConnection;
  enum class PubOrSub {
    PUBLISH,
    SUBSCRIBE
  }; ///< Internal indication whether dealing with publish or subscriber
  DataMapper* m_data_mapper;
  inline DataMapper& mapper() { return *m_data_mapper; }
  void clear(); ///< Clears and deletes all members in need of cleanup
  bool m_xml_loaded;
  eprosima::fastdds::dds::DomainParticipant* m_participant;
  eprosima::fastdds::dds::Publisher* m_publisher;
  eprosima::fastdds::dds::Subscriber* m_subscriber;
  DomainListener m_listener;
  std::map<std::string, std::string> m_topic_to_type;
  std::map<std::string, eprosima::fastrtps::types::DynamicPubSubType> m_types;
  std::map<std::string, eprosima::fastdds::dds::Topic*> m_topic_name_ptr;
  std::map<eprosima::fastdds::dds::DataReader*, eprosima::fastdds::dds::ContentFilteredTopic*>
    m_reader_topic_filter;
  std::map<eprosima::fastdds::dds::ContentFilteredTopic*, eprosima::xtypes::DynamicData&>
    m_filter_data;
  std::map<eprosima::fastdds::dds::DataWriter*, DynamicDataConnection> m_write_data;
  std::map<eprosima::fastdds::dds::DataReader*, DynamicDataConnection> m_read_data;
  ddsfmu::detail::CustomKeyFilterFactory m_filter_factory;
};

}
