#pragma once

/*
  Copyright 2023, SINTEF Ocean
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <tuple>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
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
  bool publish();
  void runPub(uint32_t samples, uint32_t sleep);
  void runSub(uint32_t periods, uint32_t sleep);
  void printInfo();
  inline uint32_t samples_received() const { return m_samples_received; }

private:
  eprosima::fastrtps::types::DynamicData_ptr m_Hello;
  eprosima::fastdds::dds::DomainParticipant* mp_participant;
  eprosima::fastdds::dds::Publisher* mp_publisher;
  eprosima::fastdds::dds::Subscriber* mp_subscriber;
  eprosima::fastdds::dds::Topic* topic_;
  eprosima::fastdds::dds::ContentFilteredTopic* filter_topic_;
  eprosima::fastdds::dds::DataWriter* writer_;
  eprosima::fastdds::dds::DataReader* reader_;
  std::unique_ptr<eprosima::xtypes::StructType> m_xtypes;
  eprosima::xtypes::idl::Context m_context;
  ddsfmu::detail::CustomKeyFilterFactory filter_factory;
  bool stop, is_publisher, has_filter;
  Permutation m_creator;
  std::uint32_t m_filter_index, m_samples_received;

  std::tuple<
    eprosima::fastrtps::types::DynamicData*,
    eprosima::fastrtps::types::DynamicPubSubType*> build_fastdds_api(eprosima::fastdds::dds::
                                                                    DomainParticipant* participant);

  std::tuple<
    eprosima::fastrtps::types::DynamicData*,
    eprosima::fastrtps::types::DynamicPubSubType*> build_xtypes_api(eprosima::fastdds::dds::
                                                                   DomainParticipant* participant);

  std::tuple<
    eprosima::fastrtps::types::DynamicData*,
    eprosima::fastrtps::types::DynamicPubSubType*> build_xtypes_idl(eprosima::fastdds::dds::
                                                                   DomainParticipant* participant);
};
