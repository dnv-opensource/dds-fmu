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

DynamicSubscriber::DynamicSubscriber()
  : mp_participant(nullptr)
  , mp_subscriber(nullptr)
  , m_listener(this)
{
}

bool DynamicSubscriber::init()
{

  auto dds_profile = (fs::current_path() / "fmu-staging" / "resources" / "config" / "dds" / "dds_profile.xml").string();

  if (efast::xmlparser::XMLP_ret::XML_OK != efast::xmlparser::XMLProfileManager::loadXMLFile(dds_profile))
  {
    std::cout << "Cannot load XML file " << dds_profile << std::endl;
    return false;
  }

  edds::StatusMask status_mask = edds::StatusMask::subscription_matched() << edds::StatusMask::data_available();
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

  // TODO: should be managed by xtypes
  std::vector<std::string> typenames{"HelloWorld", "Trig"};

  for (const auto& the_name : typenames) {

    types_[the_name] = efast::xmlparser::XMLProfileManager::getDynamicTypeByName(the_name)->build();
    edds::TypeSupport m_type(new efast::types::DynamicPubSubType(types_.at(the_name)));

    m_type.get()->auto_fill_type_information(false);  // Will not work with at least Cyclonedds if set true
    m_type.get()->auto_fill_type_object(true);        // default
    m_type.register_type(mp_participant);
  }

  rapidxml::xml_document<> doc;
  rapidxml::xml_node<> * root_node;
  std::ifstream theFile(dds_profile);
  std::vector<char> buffer(std::istreambuf_iterator<char>{theFile}, {});
  buffer.push_back('\0');
  doc.parse<0>(&buffer[0]);
  root_node = doc.first_node("ddsfmu");

  constexpr auto node_name = "fmu_out";
  for (rapidxml::xml_node<> * fmu_node = root_node->first_node(node_name);
       fmu_node;
       fmu_node = fmu_node->next_sibling(node_name))
  {

    auto topic = fmu_node->first_attribute("topic");
    auto type = fmu_node->first_attribute("type");
    if(!topic || !type){
      std::cerr << "<ddsfmu><" << node_name
                << "> must specify attributes 'topic' and 'type'. Got: 'topic': "
                << std::boolalpha << (topic != nullptr) << " and 'type': "
                << std::boolalpha << (type != nullptr) << std::endl;
      continue;
    }
    std::string topic_name(topic->value());
    std::string topic_type(type->value());

    // Create topic from profile (if it exists) otherwise default qos
    eprosima::fastdds::dds::Topic* tmp_topic = mp_participant->create_topic_with_profile(topic_name, topic_type, topic_name);
    if (tmp_topic == nullptr) {
      tmp_topic = mp_participant->create_topic(topic_name, topic_type, edds::TOPIC_QOS_DEFAULT);
    }
    if (tmp_topic == nullptr) { return false; }

    // Create reader from profile (if it exists) otherwise default qos
    eprosima::fastdds::dds::DataReader* tmp_reader = mp_subscriber->create_datareader_with_profile(tmp_topic, topic_name, &m_listener, status_mask);
    if (tmp_reader == nullptr) {
      tmp_reader = mp_subscriber->create_datareader(tmp_topic, edds::DATAREADER_QOS_DEFAULT, &m_listener, status_mask);
    }
    if (tmp_reader == nullptr) { return false; }

    topics_[tmp_reader] = tmp_topic;
    datas_[tmp_reader] = efast::types::DynamicDataFactory::get_instance()->create_data(types_.at(topic_type));
    // is TypeName retrievable from DataReader?
    // -> yes lookup from reader->type().get_type_name() gives key for types_ (not confirmed raw names)
    // not needed yet
  }

  return true;
}

DynamicSubscriber::~DynamicSubscriber()
{
  for (const auto& it : topics_)
  {
    mp_subscriber->delete_datareader(it.first);
    mp_participant->delete_topic(it.second);
  }
  if (mp_subscriber != nullptr)
  {
    mp_participant->delete_subscriber(mp_subscriber);
  }

  edds::DomainParticipantFactory::get_instance()->delete_participant(mp_participant);
  types_.clear();
  topics_.clear();
  datas_.clear();
}

void DynamicSubscriber::SubListener::on_subscription_matched(
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

void DynamicSubscriber::SubListener::on_data_available(
    edds::DataReader* reader)
{
  edds::SampleInfo info;

  // subscriber_->types_reader->type().get_type_name()

  if (reader->take_next_sample(subscriber_->datas_.at(reader).get(),
    &info) == ReturnCode_t::RETCODE_OK)
  {
    if (info.instance_state == edds::ALIVE_INSTANCE_STATE)
    {
      this->n_samples++;
      eprosima::fastrtps::types::DynamicDataHelper::print(subscriber_->datas_.at(reader));
    }
  }
}

void DynamicSubscriber::run(
    uint32_t number)
{
  std::cout << "Subscriber running until " << number << " samples have been received" << std::endl;
  std::cout << this->m_listener.n_samples << "/" << number << std::endl;
  while (number > this->m_listener.n_samples)
  {
    std::cout << this->m_listener.n_samples << "/" << number << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

int main(){

  DynamicSubscriber mysub;
  if (mysub.init())
  {
    mysub.run(5);
  }

  return 0;
}
