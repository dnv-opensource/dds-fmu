// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
// Apache-2.0

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastrtps/rtps/common/Types.h>

#include <fastrtps/types/TypeIdentifier.h>
#include <fastrtps/types/TypeObject.h>

class HelloWorldSubscriber
{
public:

  HelloWorldSubscriber();

  virtual ~HelloWorldSubscriber();

  //!Initialize the subscriber
  bool init();

  //!RUN the subscriber
  void run();

  //!Run the subscriber until number samples have been received.
  void run(uint32_t number);

private:

  eprosima::fastdds::dds::DomainParticipant* mp_participant;
  eprosima::fastdds::dds::Subscriber* mp_subscriber;
  eprosima::fastdds::dds::Topic* topic_;
  eprosima::fastdds::dds::DataReader* reader_;

public:
  eprosima::fastrtps::types::DynamicData_ptr m_Hello;
  class SubListener
    :  public eprosima::fastdds::dds::DomainParticipantListener
  {
  public:

    SubListener(
        HelloWorldSubscriber* sub)
      : n_matched(0)
      , n_samples(0)
      , subscriber_(sub)
    {
    }

    ~SubListener() override
    {
    }

    void on_data_available(
        eprosima::fastdds::dds::DataReader* reader) override;

    void on_subscription_matched(
        eprosima::fastdds::dds::DataReader* reader,
        const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

    int n_matched;
    uint32_t n_samples;
    HelloWorldSubscriber* subscriber_;

  }
    m_listener;

};
