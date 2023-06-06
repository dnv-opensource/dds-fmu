// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
// Apache-2.0

#include "pubdyntop.hpp"
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <thread>
#include <filesystem>

#include <rapidxml/rapidxml.hpp>
#include <fstream>
#include <vector>

namespace edds = eprosima::fastdds::dds;
namespace efast = eprosima::fastrtps;
namespace fs = std::filesystem;

HelloWorldPublisher::HelloWorldPublisher()
  : mp_participant(nullptr)
  , mp_publisher(nullptr)
  , writer_(nullptr)
  , topic_(nullptr)
{
}

bool HelloWorldPublisher::init()
{

  auto configs = fs::current_path() / "fmu-staging" / "resources" / "config" / "dds";
  auto dds_profile = (configs / "dds_profile.xml").string();

  if (efast::xmlparser::XMLP_ret::XML_OK !=
   efast::xmlparser::XMLProfileManager::loadXMLFile(dds_profile))
  {
    std::cout << "Cannot load XML file " << dds_profile << std::endl;
    // TODO: to be loaded once for each executable instance (i.e. once if both pub and sub in same)
    return false;
  }

  mp_participant = edds::DomainParticipantFactory::get_instance()->create_participant_with_profile("dds-fmu-default");

  if (mp_participant == nullptr) { return false; }

  mp_publisher = mp_participant->create_publisher_with_profile("dds-fmu-default");

  if (mp_publisher == nullptr) { return false; }

  // Later: Each dynamic type to be registered with participant
  // acquire and store the DynamicType_ptr (map<std::string, DynamicType_ptr>)
  // Create TypeSupport DynamicPubSubType and register the type with participant

  // TODO: replace with idl-parsed type support
  efast::types::DynamicType_ptr dyn_type =
   efast::xmlparser::XMLProfileManager::getDynamicTypeByName("HelloWorld")->build();
  edds::TypeSupport m_type(new efast::types::DynamicPubSubType(dyn_type));
  // Unclear whether this is relevant
  //m_type.get()->auto_fill_type_information(false);
  //m_type.get()->auto_fill_type_object(true);
  m_type.register_type(mp_participant);

  // For all types in idl file:
  //  register types
  //  store some dynamictype or data for populating/retrieving data

  rapidxml::xml_document<> doc;
  rapidxml::xml_node<> * root_node;
  std::ifstream theFile(dds_profile);
  std::vector<char> buffer(std::istreambuf_iterator<char>{theFile}, {});
  buffer.push_back('\0');
  doc.parse<0>(&buffer[0]);
  root_node = doc.first_node("ddsfmu");

  for (rapidxml::xml_node<> * topic_node = root_node->first_node("topic"); topic_node; topic_node = topic_node->next_sibling())
  {
    std::string topic_name(topic_node->first_attribute("name")->value());
    std::string topic_type(topic_node->first_attribute("type")->value());

    // Create topic from profile (if it exists) otherwise default qos
    topic_ = mp_participant->create_topic_with_profile(topic_name, topic_type, topic_name);
    if (topic_ == nullptr) {
      topic_ = mp_participant->create_topic(topic_name, topic_type, edds::TOPIC_QOS_DEFAULT);
    }
    if (topic_ == nullptr) { return false; }

    // Create writer from profile (if it exists) otherwise default qos
    writer_ = mp_publisher->create_datawriter_with_profile(topic_, topic_name, &m_listener);
    if (writer_ == nullptr) {
      writer_ = mp_publisher->create_datawriter(topic_, edds::DATAWRITER_QOS_DEFAULT, &m_listener);
    }
    if (writer_ == nullptr) { return false; }

  }

  // Initialise dynamic type with some data
  m_Hello = efast::types::DynamicDataFactory::get_instance()->create_data(dyn_type);
  m_Hello->set_string_value("Hello DDS Dynamic World", 0);
  m_Hello->set_uint32_value(0, 1);

  efast::types::DynamicData* array = m_Hello->loan_value(2);
  array->set_uint32_value(10, array->get_array_index({0, 0}));
  array->set_uint32_value(20, array->get_array_index({1, 0}));
  array->set_uint32_value(30, array->get_array_index({2, 0}));
  array->set_uint32_value(40, array->get_array_index({3, 0}));
  array->set_uint32_value(50, array->get_array_index({4, 0}));
  array->set_uint32_value(60, array->get_array_index({0, 1}));
  array->set_uint32_value(70, array->get_array_index({1, 1}));
  array->set_uint32_value(80, array->get_array_index({2, 1}));
  array->set_uint32_value(90, array->get_array_index({3, 1}));
  array->set_uint32_value(100, array->get_array_index({4, 1}));
  m_Hello->return_loaned_value(array);

  return true;

}

HelloWorldPublisher::~HelloWorldPublisher()
{
  if (writer_ != nullptr)
  {
    mp_publisher->delete_datawriter(writer_);
  }
  if (mp_publisher != nullptr)
  {
    mp_participant->delete_publisher(mp_publisher);
  }
  if (topic_ != nullptr)
  {
    mp_participant->delete_topic(topic_);
  }
  edds::DomainParticipantFactory::get_instance()->delete_participant(mp_participant);
}

void HelloWorldPublisher::PubListener::on_publication_matched(
    edds::DataWriter*,
    const edds::PublicationMatchedStatus& info)
{
  if (info.current_count_change == 1)
  {
    n_matched = info.total_count;
    firstConnected = true;
    std::cout << "Publisher matched" << std::endl;
  }
  else if (info.current_count_change == -1)
  {
    n_matched = info.total_count;
    std::cout << "Publisher unmatched" << std::endl;
  }
  else
  {
    std::cout << info.current_count_change
              << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
  }
}

void HelloWorldPublisher::runThread(
    uint32_t samples,
    uint32_t sleep)
{
  if (samples == 0)
  {
    while (!stop)
    {
      if (publish(false))
      {
        std::string message;
        m_Hello->get_string_value(message, 0);
        uint32_t index;
        m_Hello->get_uint32_value(index, 1);
        std::string aux_array = "[";
        efast::types::DynamicData* array = m_Hello->loan_value(2);
        for (uint32_t i = 0; i < 5; ++i)
        {
          aux_array += "[";
          for (uint32_t j = 0; j < 2; ++j)
          {
            uint32_t elem;
            array->get_uint32_value(elem, array->get_array_index({i, j}));
            aux_array += std::to_string(elem) + (j == 1 ? "]" : ", ");
          }
          aux_array += (i == 4 ? "]" : "], ");
        }
        m_Hello->return_loaned_value(array);
        std::cout << "Message: " << message << " with index: " << index
                  << " array: " << aux_array << " SENT" << std::endl;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
    }
  }
  else
  {
    for (uint32_t s = 0; s < samples; ++s)
    {
      if (!publish())
      {
        --s;
      }
      else
      {
        std::string message;
        m_Hello->get_string_value(message, 0);
        uint32_t index;
        m_Hello->get_uint32_value(index, 1);
        std::string aux_array = "[";
        efast::types::DynamicData* array = m_Hello->loan_value(2);
        for (uint32_t i = 0; i < 5; ++i)
        {
          aux_array += "[";
          for (uint32_t j = 0; j < 2; ++j)
          {
            uint32_t elem;
            array->get_uint32_value(elem, array->get_array_index({i, j}));
            aux_array += std::to_string(elem) + (j == 1 ? "]" : ", ");
          }
          aux_array += (i == 4 ? "]" : "], ");
        }
        m_Hello->return_loaned_value(array);
        std::cout << "Message: " << message << " with index: " << index
                  << " array: " << aux_array << " SENT" << std::endl;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
    }
  }
}

void HelloWorldPublisher::run(
    uint32_t samples,
    uint32_t sleep)
{
  stop = false;
  std::thread thread(&HelloWorldPublisher::runThread, this, samples, sleep);
  if (samples == 0)
  {
    std::cout << "Publisher running. Please press enter to stop the Publisher at any time." << std::endl;
    std::cin.ignore();
    stop = true;
  }
  else
  {
    std::cout << "Publisher running " << samples << " samples." << std::endl;
  }
  thread.join();
}

bool HelloWorldPublisher::publish(
    bool waitForListener)
{
  if (m_listener.firstConnected || !waitForListener || m_listener.n_matched > 0)
  {
    uint32_t index;
    m_Hello->get_uint32_value(index, 1);
    m_Hello->set_uint32_value(index + 1, 1);

    efast::types::DynamicData* array = m_Hello->loan_value(2);
    array->set_uint32_value(10 + index, array->get_array_index({0, 0}));
    array->set_uint32_value(20 + index, array->get_array_index({1, 0}));
    array->set_uint32_value(30 + index, array->get_array_index({2, 0}));
    array->set_uint32_value(40 + index, array->get_array_index({3, 0}));
    array->set_uint32_value(50 + index, array->get_array_index({4, 0}));
    array->set_uint32_value(60 + index, array->get_array_index({0, 1}));
    array->set_uint32_value(70 + index, array->get_array_index({1, 1}));
    array->set_uint32_value(80 + index, array->get_array_index({2, 1}));
    array->set_uint32_value(90 + index, array->get_array_index({3, 1}));
    array->set_uint32_value(100 + index, array->get_array_index({4, 1}));
    m_Hello->return_loaned_value(array);

    writer_->write(m_Hello.get());
    return true;
  }
  return false;
}


int main(){

  HelloWorldPublisher mypub;
  if (mypub.init())
  {
    mypub.run(10, 100);
  }

  return 0;
}
