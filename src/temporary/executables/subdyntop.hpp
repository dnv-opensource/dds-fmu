#pragma once

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastrtps/rtps/common/Types.h>

#include <fastrtps/types/TypeIdentifier.h>
#include <fastrtps/types/TypeObject.h>

#include <map>

namespace edds = eprosima::fastdds::dds;
namespace ertps = eprosima::fastrtps;

class DynamicSubscriber
{
public:
  DynamicSubscriber();
  virtual ~DynamicSubscriber();
  bool init();
  void run(uint32_t number);

private:

  ::edds::DomainParticipant* mp_participant;
  ::edds::Subscriber* mp_subscriber;

  std::map<std::string, ::ertps::types::DynamicType_ptr> types_;
  std::map<::edds::DataReader*, ::edds::Topic*> topics_;
  std::map<::edds::DataReader*, ::ertps::types::DynamicData_ptr> datas_;
  // does handle the case with multiple instances of same topic (i.e. keyed subscriptions)
  // -> value of datas_ should optionally be either DynamicData_ptr, map<keys-hash-str, DynamicData_ptr>
  // https://readthedocs.org/projects/eprosima-fast-rtps/downloads/pdf/latest/#page=237&zoom=100,96,706

public:
  class SubListener
    :  public eprosima::fastdds::dds::DomainParticipantListener
  {
  public:

    SubListener(DynamicSubscriber* sub)
      : n_matched(0)
      , n_samples(0)
      , subscriber_(sub)
    { }

    ~SubListener() override = default;

    void on_data_available(eprosima::fastdds::dds::DataReader* reader) override;

    void on_subscription_matched(
        eprosima::fastdds::dds::DataReader* reader,
        const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

    int n_matched;
    uint32_t n_samples;
    DynamicSubscriber* subscriber_;
  } m_listener;

};
