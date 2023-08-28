#pragma once

#include "DataMapper.hpp"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>

#include <fastrtps/types/DynamicPubSubType.h>

#include <filesystem>
#include <functional>
#include <map>


class DynamicPubSub
{
public:
  DynamicPubSub();
  DynamicPubSub(const DynamicPubSub&) = delete;
  DynamicPubSub& operator=(const DynamicPubSub&) = delete;
  void reset(const std::filesystem::path& fmu_resources, DataMapper* mapper);

  void write();
  void read();

private:
  enum PubOrSub { PUBLISH, SUBSCRIBE };
  eprosima::fastdds::dds::DomainParticipant* m_participant;
  eprosima::fastdds::dds::Publisher* m_publisher;
  eprosima::fastdds::dds::Subscriber* m_subscriber;
  DataMapper* m_data_mapper;
  inline DataMapper& mapper() { return *m_data_mapper; }

  bool m_xml_loaded;
  std::map<std::string, std::string> m_topic_to_type;
  std::map<std::string, eprosima::fastrtps::types::DynamicPubSubType> m_types;
  std::map<std::string, eprosima::fastdds::dds::Topic*> m_topic_name_ptr; // handled if both reader and writer?
  std::map<eprosima::fastdds::dds::DataWriter*, std::pair<eprosima::xtypes::DynamicData&, eprosima::fastrtps::types::DynamicData_ptr>> m_write_data;
  std::map<eprosima::fastdds::dds::DataReader*, std::pair<eprosima::xtypes::DynamicData&, eprosima::fastrtps::types::DynamicData_ptr>> m_read_data;

  // does handle the case with multiple instances of same topic (i.e. keyed subscriptions)
  // -> value of datas_ should optionally be either DynamicData_ptr, map<keys-hash-str, DynamicData_ptr>
  // https://readthedocs.org/projects/eprosima-fast-rtps/downloads/pdf/latest/#page=237&zoom=100,96,706


  // TODO: add PubListener



};
