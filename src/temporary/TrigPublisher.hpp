#pragma once

#include "trigPubSubTypes.h"

#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>

class TrigPublisher
{
public:
    TrigPublisher();
    virtual ~TrigPublisher();
    bool init(bool use_env);
    void publish(Trig& trig);


private:

    Trig trig_;
    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::dds::Publisher* publisher_;
    eprosima::fastdds::dds::Topic* topic_;
    eprosima::fastdds::dds::DataWriter* writer_;
    eprosima::fastdds::dds::TypeSupport type_;
};
