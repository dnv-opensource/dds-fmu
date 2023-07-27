#include "DynamicPublisher.hpp"
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

DynamicPublisher::DynamicPublisher()
  : mp_participant(nullptr)
  , mp_publisher(nullptr)
{
}

bool DynamicPublisher::init(DdsLoader& loader, const std::filesystem::path& resource_path)
{
  if (!loader.load(resource_path)) { return false; }

  mp_participant = edds::DomainParticipantFactory::get_instance()->create_participant_with_profile("dds-fmu-default");

  if (mp_participant == nullptr) { return false; }

  mp_publisher = mp_participant->create_publisher_with_profile("dds-fmu-default");

  if (mp_publisher == nullptr) { return false; }

  // The list of types to register should be retrieved from ddsfmu_mapping
  std::vector<std::string> typenames{"Trig"};

  for (const auto& the_name : typenames) {
    // TODO: replace with idl-parsed type support

    // use loader context to get DynamicType::Ptr instead
    types_[the_name] = efast::xmlparser::XMLProfileManager::getDynamicTypeByName(the_name)->build();
    edds::TypeSupport tmp_type(new efast::types::DynamicPubSubType(types_.at(the_name)));

    tmp_type.get()->auto_fill_type_information(false);  // Will not work with at least Cyclonedds if set true
    tmp_type.get()->auto_fill_type_object(true);        // default
    tmp_type.register_type(mp_participant);

  }

  // For all types in idl file:
  //  register types
  //  store some dynamictype or data for populating/retrieving data

  auto ddsfmu_mapping = resource_path / "config" / "dds" / "ddsfmu_mapping.xml";
  rapidxml::xml_document<> doc;
  rapidxml::xml_node<> * root_node;
  std::ifstream theFile(ddsfmu_mapping.string());
  std::vector<char> buffer(std::istreambuf_iterator<char>{theFile}, {});
  buffer.push_back('\0');
  doc.parse<0>(&buffer[0]);
  root_node = doc.first_node("ddsfmu");

  constexpr auto node_name = "fmu_in";
  for (rapidxml::xml_node<> * fmu_node = root_node->first_node(node_name);
       fmu_node;
       fmu_node = fmu_node->next_sibling(node_name)) {

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

    // Create writer from profile (if it exists) otherwise default qos
    eprosima::fastdds::dds::DataWriter* tmp_writer = mp_publisher->create_datawriter_with_profile(tmp_topic, topic_name, &m_listener);
    if (tmp_writer == nullptr) {
      tmp_writer = mp_publisher->create_datawriter(tmp_topic, edds::DATAWRITER_QOS_DEFAULT, &m_listener);
    }
    if (tmp_writer == nullptr) { return false; }

    topics_[tmp_writer] = tmp_topic;
    datas_[tmp_writer] = efast::types::DynamicDataFactory::get_instance()->create_data(types_.at(topic_type));

    writers_[topic_name] = tmp_writer;
  }

  return true;
}

DynamicPublisher::~DynamicPublisher()
{
  for (const auto& it : topics_) {
    mp_publisher->delete_datawriter(it.first);
    mp_participant->delete_topic(it.second);
  }
  if (mp_publisher != nullptr) {
    mp_participant->delete_publisher(mp_publisher);
  }

  edds::DomainParticipantFactory::get_instance()->delete_participant(mp_participant);
  types_.clear();
  topics_.clear();
  datas_.clear();
  writers_.clear();
}

void DynamicPublisher::PubListener::on_publication_matched(
    edds::DataWriter*,
    const edds::PublicationMatchedStatus& info)
{
  if (info.current_count_change == 1) {
    n_matched = info.total_count;
    firstConnected = true;
    std::cout << "Publisher matched" << std::endl;
  } else if (info.current_count_change == -1) {
    n_matched = info.total_count;
    std::cout << "Publisher unmatched" << std::endl;
  } else {
    std::cout << info.current_count_change
              << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
  }
}

bool DynamicPublisher::publish(
    bool waitForListener)
{
  if (m_listener.firstConnected || !waitForListener || m_listener.n_matched > 0)
  {
    auto trig = datas_.at(writers_.at("DDS_FMI_In"));
    double val;
    trig->get_float64_value(val, 0);
    trig->set_float64_value(val + 0.1, 0);
    trig->get_float64_value(val, 1);
    trig->set_float64_value(val - 0.1, 1);

    writers_.at("DDS_FMI_In")->write(trig.get());
    return true;
  }
  return false;
}

/*
void DynamicPublisher::runThread(
    uint32_t samples,
    uint32_t sleep)
{
  auto trig = datas_.at(writers_.at("DDS_FMI_In"));

  for (uint32_t s = 0; s < samples; ++s)
  {
    if (!publish()) { --s; }
    else
    {
      double sine, other;
      trig->get_float64_value(sine, 0);
      trig->get_float64_value(other, 1);
      std::cout << "Trig: " << sine << ", " << other << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
  }
}

void DynamicPublisher::run(
    uint32_t samples,
    uint32_t sleep)
{

  auto trig = datas_.at(writers_.at("DDS_FMI_In"));
  trig->set_float64_value(1., 0);
  trig->set_float64_value(2., 1);

  std::thread thread(&DynamicPublisher::runThread, this, samples, sleep);
  std::cout << "Publisher running " << samples << " samples." << std::endl;
  thread.join();
}
*/
