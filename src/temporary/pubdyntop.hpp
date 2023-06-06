#pragma once

// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
// Apache-2.0

#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>

class HelloWorldPublisher
{
public:

  HelloWorldPublisher();

  virtual ~HelloWorldPublisher();

  //!Initialize
  bool init();

  //!Publish a sample
  bool publish(
      bool waitForListener = true);

  //!Run for number samples
  void run(
      uint32_t number,
      uint32_t sleep);

private:

  eprosima::fastrtps::types::DynamicData_ptr m_Hello;
  eprosima::fastdds::dds::DomainParticipant* mp_participant;
  eprosima::fastdds::dds::Publisher* mp_publisher;
  eprosima::fastdds::dds::Topic* topic_;
  eprosima::fastdds::dds::DataWriter* writer_;
  bool stop;

  class PubListener : public eprosima::fastdds::dds::DataWriterListener
  {
  public:

    PubListener()
      : n_matched(0)
      , firstConnected(false)
    {
    }

    ~PubListener() override
    {
    }

    void on_publication_matched(
        eprosima::fastdds::dds::DataWriter* writer,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;
    // should override on_offered_incompatible_qos too
    int n_matched;
    bool firstConnected;

  }
    m_listener;

  void runThread(
      uint32_t number,
      uint32_t sleep);
};
