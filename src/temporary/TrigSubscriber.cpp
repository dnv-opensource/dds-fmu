

#include "TrigSubscriber.hpp"
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>

using namespace eprosima::fastdds::dds;

TrigSubscriber::TrigSubscriber()
    : participant_(nullptr)
    , subscriber_(nullptr)
    , topic_(nullptr)
    , reader_(nullptr)
    , type_(new TrigPubSubType())
{
}

bool TrigSubscriber::init(
        bool use_env)
{
    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
    pqos.name("Participant_sub");
    auto factory = DomainParticipantFactory::get_instance();

    // https://fast-dds.docs.eprosima.com/en/latest/fastdds/transport/disabling_multicast.html
    // Metatraffic Multicast Locator List will be empty.
    // Metatraffic Unicast Locator List will contain one locator, with null address and null port.
    // Then Fast DDS will use all network interfaces to receive network messages using a well-known port.
    eprosima::fastrtps::rtps::Locator_t default_unicast_locator;
    pqos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(default_unicast_locator);

    // Initial peer will be UDPv4 address below. The port will be a well-known port.
    // Initial discovery network messages will be sent to this UDPv4 address.
    eprosima::fastrtps::rtps::Locator_t initial_peer;
    eprosima::fastrtps::rtps::IPLocator::setIPv4(initial_peer, 10, 2, 0, 155);
    pqos.wire_protocol().builtin.initialPeersList.push_back(initial_peer);


    if (use_env)
    {
        factory->load_profiles();
        factory->get_default_participant_qos(pqos);
    }

    participant_ = factory->create_participant(1, pqos);

    if (participant_ == nullptr)
    {
        return false;
    }

    //REGISTER THE TYPE
    type_.register_type(participant_);

    //CREATE THE SUBSCRIBER
    SubscriberQos sqos = SUBSCRIBER_QOS_DEFAULT;

    if (use_env)
    {
        participant_->get_default_subscriber_qos(sqos);
    }

    subscriber_ = participant_->create_subscriber(sqos, nullptr);

    if (subscriber_ == nullptr)
    {
        return false;
    }

    //CREATE THE TOPIC
    TopicQos tqos = TOPIC_QOS_DEFAULT;

    if (use_env)
    {
        participant_->get_default_topic_qos(tqos);
    }

    topic_ = participant_->create_topic(
        "DDS_FMI_Out",
        "Trig",
        tqos);

    if (topic_ == nullptr)
    {
        return false;
    }

    // CREATE THE READER
    DataReaderQos rqos = DATAREADER_QOS_DEFAULT;
    rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    if (use_env)
    {
        subscriber_->get_default_datareader_qos(rqos);
    }

    reader_ = subscriber_->create_datareader(topic_, rqos);

    if (reader_ == nullptr)
    {
        return false;
    }

    return true;
}

TrigSubscriber::~TrigSubscriber()
{
    if (reader_ != nullptr)
    {
        subscriber_->delete_datareader(reader_);
    }
    if (topic_ != nullptr)
    {
        participant_->delete_topic(topic_);
    }
    if (subscriber_ != nullptr)
    {
        participant_->delete_subscriber(subscriber_);
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

Trig& TrigSubscriber::read()
{
  SampleInfo info;
  reader_->take_next_sample(&trig_, &info);
  return trig_;

}
