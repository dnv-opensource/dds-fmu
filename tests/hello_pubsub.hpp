#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <map>
#include <mutex>
#include <thread>
#include <tuple>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/topic/ContentFilteredTopic.hpp> // Cannot be used due to https://github.com/eProsima/Fast-DDS/issues/3296
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/rtps/common/Types.h>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicDataHelper.hpp>
#include <fastrtps/types/DynamicPubSubType.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>
#include <fastrtps/types/DynamicTypePtr.h>
#include <fastrtps/types/TypeIdentifier.h>
#include <fastrtps/types/TypeObject.h>
#include <xtypes/idl/idl.hpp>
#include <xtypes/xtypes.hpp>

#include "Converter.hpp"
#include "CustomKeyFilterFactory.hpp"


enum class Permutation { API_FASTDDS, API_XTYPES, API_XTYPES_IDL };

class HelloPubSub {
public:
  HelloPubSub() = delete;
  HelloPubSub(Permutation creator, bool pub = true, std::uint32_t filter_index = 0, bool with_filter=false);
  virtual ~HelloPubSub();
  bool init();
  bool publish(bool waitForListener = true);
  void runPub(uint32_t samples, uint32_t sleep);
  void runThread(uint32_t samples, uint32_t sleep);
  void printInfo();
  inline uint32_t samples_received() const { return m_sub_listener.n_samples; }

private:
  eprosima::fastrtps::types::DynamicData_ptr m_Hello;
  eprosima::fastdds::dds::DomainParticipant* mp_participant;
  eprosima::fastdds::dds::Publisher* mp_publisher;
  eprosima::fastdds::dds::Subscriber* mp_subscriber;
  eprosima::fastdds::dds::Topic* topic_;
  eprosima::fastdds::dds::ContentFilteredTopic* filter_topic_;
  eprosima::fastdds::dds::DataWriter* writer_;
  eprosima::fastdds::dds::DataReader* reader_;
  ddsfmu::detail::CustomKeyFilterFactory filter_factory;
  bool stop, is_publisher, has_filter;
  Permutation m_creator;
  std::uint32_t m_filter_index;

  std::tuple<
    eprosima::fastrtps::types::DynamicData*,
    eprosima::fastrtps::types::DynamicPubSubType*> static build_fastdds_api(eprosima::fastdds::dds::
                                                                    DomainParticipant* participant);

  std::tuple<
    eprosima::fastrtps::types::DynamicData*,
    eprosima::fastrtps::types::DynamicPubSubType*> static build_xtypes_api(eprosima::fastdds::dds::
                                                                   DomainParticipant* participant);

  std::tuple<
    eprosima::fastrtps::types::DynamicData*,
    eprosima::fastrtps::types::DynamicPubSubType*> static build_xtypes_idl(eprosima::fastdds::dds::
                                                                   DomainParticipant* participant);

  class PubListener : public eprosima::fastdds::dds::DataWriterListener {
  public:
    PubListener() : n_matched(0), firstConnected(false) {}
    ~PubListener() override {}
    void on_publication_matched(
      eprosima::fastdds::dds::DataWriter* writer,
      const eprosima::fastdds::dds::PublicationMatchedStatus& info) override {
      if (info.current_count_change == 1) {
        n_matched = info.total_count;
        firstConnected = true;
        std::cout << "Publisher matched" << std::endl;
      } else if (info.current_count_change == -1) {
        n_matched = info.total_count;
        std::cout << "Publisher unmatched" << std::endl;
      } else {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change"
                  << std::endl;
      }
    }
    int n_matched;
    bool firstConnected;
  } m_pub_listener;

public:
  class SubListener : public eprosima::fastdds::dds::DomainParticipantListener {
  public:
    SubListener(HelloPubSub* sub) : n_matched(0), n_samples(0), subscriber_(sub) {}
    ~SubListener() override {}

    void on_data_available(eprosima::fastdds::dds::DataReader* reader) override {
      if (subscriber_->reader_ != reader) {
        std::cout << "Not the reader you are looking for " << std::endl;
        return;
      }

      eprosima::fastdds::dds::SampleInfo info;

      if (
        reader->take_next_sample(subscriber_->m_Hello.get(), &info)
        == eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK) {
        if (info.valid_data) { std::cout << "Got new valid data" << std::endl; }

        //subscriber_->m_Hello.get()

        if (info.instance_state == eprosima::fastdds::dds::ALIVE_INSTANCE_STATE) {
          this->n_samples++;
          std::cout << "Yep" << std::endl;
          //eprosima::fastrtps::types::DynamicDataHelper::print(subscriber_->m_Hello);
          std::cout << std::endl;
        }
      }
    }

    void on_subscription_matched(
      eprosima::fastdds::dds::DataReader* reader,
      const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override {
      if (info.current_count_change == 1) {
        n_matched = info.total_count;
        std::cout << "Subscriber matched" << std::endl;
      } else if (info.current_count_change == -1) {
        n_matched = info.total_count;
        std::cout << "Subscriber unmatched" << std::endl;
      } else {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change"
                  << std::endl;
      }
    }
    int n_matched;
    uint32_t n_samples;
    HelloPubSub* subscriber_;
    eprosima::fastdds::dds::InstanceHandle_t previous_handle;
    std::map<eprosima::fastdds::dds::InstanceHandle_t, uint32_t> handles;
  } m_sub_listener;
};
