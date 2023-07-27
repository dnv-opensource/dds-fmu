
#include "TrigPublisher.hpp"
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>

using namespace eprosima::fastdds::dds;

TrigPublisher::TrigPublisher()
    : participant_(nullptr)
    , publisher_(nullptr)
    , topic_(nullptr)
    , writer_(nullptr)
    , type_(new TrigPubSubType())
{
}

bool TrigPublisher::init(
        bool use_env)
{
    trig_.sine(0);
    trig_.other(1);
    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
    pqos.name("Participant_pub");
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

    //CREATE THE PUBLISHER
    PublisherQos pubqos = PUBLISHER_QOS_DEFAULT;

    if (use_env)
    {
        participant_->get_default_publisher_qos(pubqos);
    }

    publisher_ = participant_->create_publisher(
        pubqos,
        nullptr);

    if (publisher_ == nullptr)
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
        "DDS_FMI_In",
        "Trig",
        tqos);

    if (topic_ == nullptr)
    {
        return false;
    }

    // CREATE THE WRITER
    DataWriterQos wqos = DATAWRITER_QOS_DEFAULT;

    if (use_env)
    {
        publisher_->get_default_datawriter_qos(wqos);
    }

    writer_ = publisher_->create_datawriter(
        topic_,
        wqos);

    if (writer_ == nullptr)
    {
        return false;
    }

    return true;
}

TrigPublisher::~TrigPublisher()
{
    if (writer_ != nullptr)
    {
        publisher_->delete_datawriter(writer_);
    }
    if (publisher_ != nullptr)
    {
        participant_->delete_publisher(publisher_);
    }
    if (topic_ != nullptr)
    {
        participant_->delete_topic(topic_);
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

void TrigPublisher::publish(Trig& trig)
{
  writer_->write(&trig);
}
