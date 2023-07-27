#pragma once

#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>

#include <map>

namespace edds = eprosima::fastdds::dds;
namespace ertps = eprosima::fastrtps;

class DynamicPublisher
{
public:

  DynamicPublisher();
  virtual ~DynamicPublisher();
  bool init();
  bool publish(bool waitForListener = true);
  void run(uint32_t number, uint32_t sleep);

private:
  ::edds::DomainParticipant* mp_participant;
  ::edds::Publisher* mp_publisher;

  std::map<std::string, ::ertps::types::DynamicType_ptr> types_;
  std::map<::edds::DataWriter*, ::edds::Topic*> topics_;
  std::map<::edds::DataWriter*, ::ertps::types::DynamicData_ptr> datas_;
  std::map<std::string, ::edds::DataWriter*> writers_;

  class PubListener : public eprosima::fastdds::dds::DataWriterListener
  {
  public:

    PubListener()
      : n_matched(0)
      , firstConnected(false)
    { }

    ~PubListener() override = default;

    void on_publication_matched(
        eprosima::fastdds::dds::DataWriter* writer,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

    // should override on_offered_incompatible_qos too

    int n_matched;
    bool firstConnected;

  } m_listener;

  void runThread(
      uint32_t number,
      uint32_t sleep);
};
