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


/**
   @brief Dynamic Publisher and Subscriber

   This class consists of one each: Domain Participant, dds::Publisher, and dds::Subscriber.
   It loads a Fast-DDS XML profile, uses IDL files for type specification, and a DDS to FMU mapping configuration file.
   For each mapping of DDS topic, it registers either a DataWriter or DataReader entity.
   It has convenience functions to call write or take on all registered entities.
   By means of a converter, the inbound or outbound DDS data are populated in a connected DataMapper instance.

*/
class DynamicPubSub
{
public:
  DynamicPubSub(); ///< Default constructor sets pointers to nullptr and m_xml_load ed false
  ~DynamicPubSub(); ///< Destructor calls clear()
  DynamicPubSub(const DynamicPubSub&) = delete; ///< Copy constructor
  DynamicPubSub& operator=(const DynamicPubSub&) = delete; ///< Copy assignment

  /**
     @brief Resets all members of DynamicPubSub

     The function loads configuration files and initializes DDS members, as well as other data structures.


     @param [in] fmu_resourcs Path to FMU resources folder
     @param [in] mapper Pointer to DataMapper instance to be used

  */
  void reset(const std::filesystem::path& fmu_resources, DataMapper* const mapper);

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

private:
  void clear(); ///< Clears and deletes all members in need of cleanup
  typedef std::pair<eprosima::xtypes::DynamicData&, eprosima::fastrtps::types::DynamicData_ptr> DynamicDataConnection;
  enum PubOrSub { PUBLISH, SUBSCRIBE }; ///< Internal indication whether dealing with publish or subscriber
  eprosima::fastdds::dds::DomainParticipant* m_participant;
  eprosima::fastdds::dds::Publisher* m_publisher;
  eprosima::fastdds::dds::Subscriber* m_subscriber;
  DataMapper* m_data_mapper;
  inline DataMapper& mapper() { return *m_data_mapper; }
  bool m_xml_loaded;
  std::map<std::string, std::string> m_topic_to_type;
  std::map<std::string, eprosima::fastrtps::types::DynamicPubSubType> m_types;
  std::map<std::string, eprosima::fastdds::dds::Topic*> m_topic_name_ptr;
  std::map<eprosima::fastdds::dds::DataWriter*, DynamicDataConnection> m_write_data;
  std::map<eprosima::fastdds::dds::DataReader*, DynamicDataConnection> m_read_data;

  // m_{read,write}_data (read?) also need to distinguish data for same topic in case of @key (separate data instances)
  // https://readthedocs.org/projects/eprosima-fast-rtps/downloads/pdf/latest/#page=237&zoom=100,96,706

};
