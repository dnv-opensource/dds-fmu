// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
// Apache-2.0

#include "subdyntop.hpp"
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

#include <fastrtps/types/DynamicDataHelper.hpp>
#include <fastrtps/types/DynamicDataFactory.h>
#include <filesystem>

#include <rapidxml/rapidxml.hpp>
#include <fstream>
#include <vector>

namespace edds = eprosima::fastdds::dds;
namespace efast = eprosima::fastrtps;
namespace fs = std::filesystem;
using eprosima::fastrtps::types::ReturnCode_t;

HelloWorldSubscriber::HelloWorldSubscriber()
  : mp_participant(nullptr)
  , mp_subscriber(nullptr)
  , m_listener(this)
{
}

bool HelloWorldSubscriber::init()
{

  auto dds_profile = (fs::current_path() / "fmu-staging" / "resources" / "config" / "dds" / "dds_profile.xml").string();

  if (efast::xmlparser::XMLP_ret::XML_OK != efast::xmlparser::XMLProfileManager::loadXMLFile(dds_profile))
  {
    std::cout << "Cannot load XML file " << dds_profile << std::endl;
    return false;
  }

  edds::StatusMask status_mask = edds::StatusMask::subscription_matched() << edds::StatusMask::data_available();
  //mp_participant = edds::DomainParticipantFactory::get_instance()->create_participant(0, pqos, &m_listener, status_mask);
  mp_participant = edds::DomainParticipantFactory::get_instance()->create_participant_with_profile("dds-fmu-default", &m_listener, status_mask);


  if (mp_participant == nullptr)
  {
    return false;
  }

  if (mp_participant->enable() != ReturnCode_t::RETCODE_OK)
  {
    edds::DomainParticipantFactory::get_instance()->delete_participant(mp_participant);
    return false;
  }

  mp_subscriber = mp_participant->create_subscriber_with_profile("dds-fmu-default");

  if (mp_subscriber == nullptr)
  {
    return false;
  }

  efast::types::DynamicType_ptr dyn_type =
   efast::xmlparser::XMLProfileManager::getDynamicTypeByName("HelloWorld")->build();

  edds::TypeSupport m_type(new efast::types::DynamicPubSubType(dyn_type));
  m_Hello = efast::types::DynamicDataFactory::get_instance()->create_data(dyn_type);
  m_type.register_type(mp_participant);

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

    // Create reader from profile (if it exists) otherwise default qos
    reader_ = mp_subscriber->create_datareader_with_profile(topic_, topic_name, &m_listener, status_mask);
    if (reader_ == nullptr) {
      reader_ = mp_subscriber->create_datareader(topic_, edds::DATAREADER_QOS_DEFAULT, &m_listener, status_mask);
    }
    if (reader_ == nullptr) { return false; }

  }

  return true;
}

HelloWorldSubscriber::~HelloWorldSubscriber()
{
  std::cout << "Destructing" << std::endl;
  if (reader_ != nullptr)
  {
    mp_subscriber->delete_datareader(reader_);
  }
  if (mp_subscriber != nullptr)
  {
    mp_participant->delete_subscriber(mp_subscriber);
  }
  if (topic_ != nullptr)
  {
    mp_participant->delete_topic(topic_);
  }
  edds::DomainParticipantFactory::get_instance()->delete_participant(mp_participant);
}

void HelloWorldSubscriber::SubListener::on_subscription_matched(
    edds::DataReader*,
    const edds::SubscriptionMatchedStatus& info)
{
  if (info.current_count_change == 1)
  {
    n_matched = info.total_count;
    std::cout << "Subscriber matched" << std::endl;
  }
  else if (info.current_count_change == -1)
  {
    n_matched = info.total_count;
    std::cout << "Subscriber unmatched" << std::endl;
  }
  else
  {
    std::cout << info.current_count_change
              << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
  }
}

void HelloWorldSubscriber::SubListener::on_data_available(
    edds::DataReader* reader)
{
  edds::SampleInfo info;

  // map<reader,dynamicdata_ptr> -> lookup and populate instead of hard-coded m_Hello

  if (reader->take_next_sample(subscriber_->m_Hello.get(), &info) == ReturnCode_t::RETCODE_OK)
  {
    if (info.instance_state == edds::ALIVE_INSTANCE_STATE)
    {
      this->n_samples++;
      eprosima::fastrtps::types::DynamicDataHelper::print(subscriber_->m_Hello);
    }
  }
}

void HelloWorldSubscriber::run()
{
  std::cout << "Subscriber running. Please press CTRL-C to stop the Subscriber" << std::endl;
  std::cin.ignore();
}

void HelloWorldSubscriber::run(
    uint32_t number)
{
  std::cout << "Subscriber running until " << number << " samples have been received" << std::endl;
  std::cout << this->m_listener.n_samples << "/" << number << std::endl;
  while (number > this->m_listener.n_samples)
  {
    std::cout << this->m_listener.n_samples << "/" << number << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

int main(){

  HelloWorldSubscriber mysub;
  if (mysub.init())
  {
    mysub.run(5);
  }

  return 0;
}
