//#include <functional>
//#include <vector>

#include <chrono>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <utility>

#include <gtest/gtest.h>
#include <xtypes/idl/idl.hpp>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>

#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>

#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicPubSubType.h>
#include <fastrtps/types/DynamicTypePtr.h>

#include "Converter.hpp"

class MyReaderListener : public eprosima::fastdds::dds::DataReaderListener
{
public:
  ~MyReaderListener() final = default;
  MyReaderListener() = delete;
  MyReaderListener(
      eprosima::fastrtps::types::DynamicData* dynamic_data,
      eprosima::xtypes::DynamicData& message)
    : m_dynamic_data(dynamic_data), m_message(message) {}
  void on_data_available(
      eprosima::fastdds::dds::DataReader* reader) override
  {
    //(void)reader;
    eprosima::fastdds::dds::SampleInfo info;
    if (eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK
     == reader->take_next_sample(m_dynamic_data, &info)) {
      ddsfmu::Converter::fastdds_to_xtypes(m_dynamic_data, m_message);
    }

  }
private:
   eprosima::fastrtps::types::DynamicData* m_dynamic_data;
   eprosima::xtypes::DynamicData& m_message;
  // should reader pointer be attached here too?
};



TEST(PubSub, UsageProcedure)
{
  // The following pseudo-code based on https://github.com/eProsima/FastDDS-SH/blob/main/src/{Publisher|Subscriber|Participant}.{hpp,cpp}
  // Conversion scenario for DDS is as follows
  eprosima::fastdds::dds::DomainParticipant* participant;
  eprosima::fastdds::dds::Publisher* publisher;
  eprosima::fastdds::dds::Subscriber* subscriber;

  eprosima::fastdds::dds::DomainParticipantQos participant_qos = eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT;

  eprosima::fastdds::dds::DomainId_t domain_id(0);
  participant = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(domain_id, participant_qos);

  ASSERT_NE(participant, nullptr);

  publisher = participant->create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT);

  subscriber = participant->create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT);

  ASSERT_NE(publisher, nullptr) << "Successful creation of publisher";
  ASSERT_NE(subscriber, nullptr) << "Successful creation of subscriber";

  std::string my_idl = R"~~~(
    struct Basic
    {
        uint32 basically_unsigned;
    };

    module Intermediate
    {
       struct Complexity
       {
           string str;
       };
    };

  )~~~";

  // 1. load idl with types into xtypes
  eprosima::xtypes::idl::Context context = eprosima::xtypes::idl::parse(my_idl);

  // 2. user requests (topic_name, type_name) to be Pub|Sub
  std::string topic_name("my_topic"), type_name("Intermediate::Complexity");

  ASSERT_TRUE(context.module().has_structure(type_name)) << "IDL has requested type";
  const eprosima::xtypes::DynamicType& message_type(context.module().structure(type_name));

  // 3. retrieve DynamicTypeBuilder* bldr from Converter::create_builder(message_type)
  eprosima::fastrtps::types::DynamicTypeBuilder* builder = ddsfmu::Converter::create_builder(message_type);

  // 4. register dynamic type with participant, by providing topic_name, type_name.name(), bldr)

  // a. To add map<topic,type>, map<type, DynamicPubSubType>, and map<Topic*, DomainEntity*>
  std::map<std::string, std::string> topic_to_type;
  std::map<std::string, eprosima::fastrtps::types::DynamicPubSubType> types;

  // b. if exists in map<topic,type> return (registered)
  if (topic_to_type.find(topic_name) != topic_to_type.end()) {
    return; // topic registered (and hence type)
  }

  // c. if exists in map<type, pubsubtype>, type known, register in b. return
  if (types.find(type_name) != types.end()) {
    topic_to_type.emplace(topic_name, type_name);
    return; // type registered
  }

  // d. dyntypptr: build DynamicType_ptr with builder
  eprosima::fastrtps::types::DynamicType_ptr dyntypptr = builder->build();

  ASSERT_NE(dyntypptr, nullptr) << "Create DynamicType_ptr";

  // e add to types map
  auto pair = types.emplace(type_name, eprosima::fastrtps::types::DynamicPubSubType(dyntypptr));

  bool added = pair.second;
  eprosima::fastrtps::types::DynamicPubSubType& dynamic_type_support = pair.first->second;
  topic_to_type.emplace(topic_name, type_name);

  eprosima::fastdds::dds::TypeSupport p_type = participant->find_type(type_name);

  // f. check if already registered with dds participant
  if(p_type == nullptr ) { /* not registered */
    dynamic_type_support.setName(type_name.c_str());
    /* A bug with UnionType in Fast DDS Dynamic Types is bypassed. */
    // WORKAROUND START
    dynamic_type_support.auto_fill_type_information(false);
    dynamic_type_support.auto_fill_type_object(false); // TODO: or true?
    // WORKAROUND END

    ASSERT_FALSE(added && !participant->register_type(dynamic_type_support))
     << "Register dynamic type support with participant";
  }

  if(added) {
    // Why registered?
    ddsfmu::Converter::register_type(topic_name, &dynamic_type_support);
  }

  // 5. Given a topic_name, create dynamic data DynamicData* dynamic_data;

  const eprosima::fastrtps::types::DynamicType_ptr& dynamic_type =
   types.at(topic_to_type.at(topic_name)).GetDynamicType();

  eprosima::fastrtps::types::DynamicData* dynamic_data_pub =
   eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(dynamic_type);

  eprosima::fastrtps::types::DynamicData* dynamic_data_sub =
   eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(dynamic_type);

  // 6. Create topic (TODO: dds_participant->lookup_topicdescription(topic_name) is it because a participant can have e.g. both reader and writer with a topic? it returns something that can static_cast<Topic*>(ret));

  eprosima::fastdds::dds::Topic* topic = participant->create_topic(
      topic_name, type_name, eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);

  ASSERT_NE(topic, nullptr) << "Creating topic with name and type";

  // 7. Create datawriter|datareader

  eprosima::fastdds::dds::DataWriter* datawriter = publisher->create_datawriter(
      topic, eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT);

  eprosima::fastdds::dds::DataReader* datareader = subscriber->create_datareader(
      topic, eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT);


  ASSERT_NE(datawriter, nullptr) << "Create data writer for topic";
  ASSERT_NE(datareader, nullptr) << "Create data reader for topic";

  eprosima::xtypes::DynamicData received_message(message_type);

  MyReaderListener listen(dynamic_data_sub, received_message);
  datareader->set_listener(&listen);

  /*
    8 a. How to publish
    - Starting point should be a const xtypes::DynamicData& message to be sent
    - Convert to dds-compatible dynamicdata: Converter::xtypes_to_fastdds(message, dynamic_data)
    - datawriter->writer(static_cast<void*>(dynamic_data))
  */
  eprosima::xtypes::DynamicData sent_message(message_type);
  //sent_message["basically_unsigned"] = 33u;
  sent_message["str"] = "hello";
  ddsfmu::Converter::xtypes_to_fastdds(sent_message, dynamic_data_pub);
  datawriter->write(static_cast<void*>(dynamic_data_pub));

  /*
    b. How to subscribe
    - a receiver function with arguments (const DynamicData* data, dds::SampleInfo info) is called from on_data_available
    - here, data is dynamic_data_, previously reader->take_next_sample(dynamic_data_, &info)
    - use member variable const xtypes::DynamicType& message_type to
    create a local xtypes::DynamicData data_copy(message_type)
    - copy data with Converter::fastdds_to_xtypes(data, data_copy)
    - For subscriber a new thread for each on_data_available is spawned calling recieve. With misc. condition variables, and cleaners.
  */

  // Option 1: callback-based read
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  EXPECT_EQ(sent_message, received_message) << "Callback-based reading of message";


  // Option 2: poll-based read
  //sent_message["basically_unsigned"] = 44u;
  sent_message["str"] = "bye";
  EXPECT_NE(sent_message, received_message) << "The messages are not reference to the same";

  datareader->set_listener(nullptr); // disable callback
  ddsfmu::Converter::xtypes_to_fastdds(sent_message, dynamic_data_pub);
  datawriter->write(static_cast<void*>(dynamic_data_pub));
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  eprosima::fastdds::dds::SampleInfo info;
  if (eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK
   == datareader->take_next_sample(dynamic_data_sub, &info)) {
    ddsfmu::Converter::fastdds_to_xtypes(dynamic_data_sub, received_message);
  }

  EXPECT_EQ(sent_message, received_message) << "Poll-based read";
  std::cout << received_message["str"].value<std::string>() << std::endl;

  // Clean-up
  eprosima::fastrtps::types::DynamicDataFactory::get_instance()->delete_data(dynamic_data_pub);
  eprosima::fastrtps::types::DynamicDataFactory::get_instance()->delete_data(dynamic_data_sub);

  datawriter->set_listener(nullptr);
  publisher->delete_datawriter(datawriter);
  participant->delete_publisher(publisher);

  datareader->set_listener(nullptr);
  subscriber->delete_datareader(datareader);
  participant->delete_subscriber(subscriber);

  participant->delete_topic(topic);
  eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant);

}
