#include "hello_pubsub.hpp"

#include "Converter.hpp"

HelloPubSub::HelloPubSub(Permutation creator, bool pub, std::uint32_t filter_index, bool with_filter)
    : mp_participant(nullptr)
    , reader_(nullptr)
    , writer_(nullptr)
    , topic_(nullptr)
    , filter_topic_(nullptr)
    , mp_publisher(nullptr)
    , mp_subscriber(nullptr)
    , is_publisher(pub)
    , has_filter(with_filter)
    , m_creator(creator)
    , m_sub_listener(this)
    , m_filter_index(filter_index) {}

HelloPubSub::~HelloPubSub() {
  if (reader_ != nullptr) { mp_subscriber->delete_datareader(reader_); }
  if (writer_ != nullptr) { mp_publisher->delete_datawriter(writer_); }
  if (mp_subscriber != nullptr) { mp_participant->delete_subscriber(mp_subscriber); }
  if (mp_publisher != nullptr) { mp_participant->delete_publisher(mp_publisher); }
  if (topic_ != nullptr) { mp_participant->delete_topic(topic_); }
  if (filter_topic_ != nullptr) { mp_participant->delete_contentfilteredtopic(filter_topic_); }

  mp_participant->delete_contained_entities();

  eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(
    mp_participant);
}

bool HelloPubSub::init() {
  eprosima::fastdds::dds::DomainParticipantQos pqos;
  if (is_publisher) {
    pqos.name("Participant_pub");
    mp_participant =
      eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(0, pqos);
  } else {
    pqos.name("Participant_sub");
    eprosima::fastdds::dds::StatusMask par_mask =
      eprosima::fastdds::dds::StatusMask::subscription_matched()
      << eprosima::fastdds::dds::StatusMask::data_available();

    mp_participant =
      eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
        0, pqos, &m_sub_listener, par_mask);
  }

  if (mp_participant == nullptr) { return false; }

  // Support various ways of creating m_Hello
  // struct HelloWorld
  // {
  //   unsigned long index;
  //   string message;
  //   @key MyEnum enumber;
  // };

  eprosima::fastrtps::types::DynamicPubSubType* my_type;
  switch (m_creator) {
  case Permutation::API_FASTDDS: {
    auto answer = build_fastdds_api(mp_participant); // Option 1: using fast-dds API
    m_Hello = std::get<0>(answer);
    my_type = std::get<1>(answer);
    break;
  }
  case Permutation::API_XTYPES: {
    auto answer = build_xtypes_api(mp_participant); // Option 2: using xtypes API
    m_Hello = std::get<0>(answer);
    my_type = std::get<1>(answer);
    break;
  }
  case Permutation::API_XTYPES_IDL: {
    auto answer = build_xtypes_idl(mp_participant); // Option 3: using xtypes IDL API
    m_Hello = std::get<0>(answer);
    my_type = std::get<1>(answer);
    break;
  }
  default: throw std::logic_error("Not implemented");
  }

  m_Hello->set_uint32_value(1, 0);
  m_Hello->set_string_value("Hello Dynamic", 1);


  //CREATE THE TOPIC
  std::string topic_name("HelloTopic");
  topic_ = mp_participant->create_topic(
    topic_name, "HelloWorld", eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
  if (topic_ == nullptr) { return false; }

  if (is_publisher) {
    //CREATE THE PUBLISHER
    mp_publisher =
      mp_participant->create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT, nullptr);
    if (mp_publisher == nullptr) { return false; }

    // CREATE THE WRITER
    writer_ = mp_publisher->create_datawriter(
      topic_, eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT, &m_pub_listener);
    if (writer_ == nullptr) { return false; }

  } else {
    // CREATE THE SUBSCRIBER
    mp_subscriber =
      mp_participant->create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT, nullptr);

    if (mp_subscriber == nullptr) { return false; }


    // Register custom content filter factory
    if (
      eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK
      != mp_participant->register_content_filter_factory("CUSTOM_KEY_FILTER", &filter_factory)) {
      std::cout << "Factory already registered?" << std::endl;
    }

    if (has_filter) {
      filter_topic_ = mp_participant->create_contentfilteredtopic(
          topic_name + "Filtered", topic_, " ", {"|GUID UNKNOWN|", std::to_string(m_filter_index)}, "CUSTOM_KEY_FILTER");

      if (filter_topic_ == nullptr) {
        std::cout << "Unable to create filtered topic" << std::endl;
        throw std::runtime_error("Filter failure");
      }
    }

    // CREATE THE COMMON READER ATTRIBUTES
    eprosima::fastdds::dds::DataReaderQos qos = eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT;
    qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;

    eprosima::fastdds::dds::StatusMask sub_mask =
      eprosima::fastdds::dds::StatusMask::subscription_matched()
      << eprosima::fastdds::dds::StatusMask::data_available();

    // should know topic is keyed or not at some earlier point

    if (filter_topic_) {
      reader_ = mp_subscriber->create_datareader(filter_topic_, qos, &m_sub_listener, sub_mask);
    } else {
      reader_ = mp_subscriber->create_datareader(topic_, qos, &m_sub_listener, sub_mask);
    }

    if (reader_ == nullptr) { return false; }

    if(has_filter) {
      // Update expression parameters with reader GUID
      std::stringstream oss;
      oss << reader_->guid();
      std::vector<std::string> old_params;
      filter_topic_->get_expression_parameters(old_params);
      old_params[0] = oss.str();
      filter_topic_->set_expression_parameters(old_params);
    }
  }

  return true;
}

void HelloPubSub::printInfo() {
  if(filter_topic_) {
    std::cout << "ContentFilteredTopic info for '" << filter_topic_->get_name() << "':" << std::endl;
    std::cout << "  - Related Topic: " << filter_topic_->get_related_topic()->get_name() << std::endl;
  }
}

  bool HelloPubSub::publish(bool waitForListener) {
    if (!is_publisher) { return false; }
    if (m_pub_listener.firstConnected || !waitForListener || m_pub_listener.n_matched > 0) {
      uint32_t index;
      m_Hello->get_uint32_value(index, 0);
      m_Hello->set_uint32_value(index + 1, 0);
      if(index % 2) {
        m_Hello->set_enum_value("BETA", 2);
      } else {
        m_Hello->set_enum_value("ALPHA", 2);
      }

      writer_->write(m_Hello.get());
      return true;
    }
    return false;
  }
  void HelloPubSub::runPub(uint32_t samples, uint32_t sleep) {
    if (!is_publisher) { return; }
    stop = false;
    std::thread thread(&HelloPubSub::runThread, this, samples, sleep);
    if (samples == 0) {
      std::cout << "Publisher running. Please press enter to stop the Publisher at any time."
                << std::endl;
      std::cin.ignore();
      stop = true;
    } else {
      std::cout << "Publisher running " << samples << " samples." << std::endl;
    }
    thread.join();
  }
  void HelloPubSub::runThread(uint32_t samples, uint32_t sleep) {
    if (!is_publisher) { return; }
    for (uint32_t s = 0; s < samples; ++s) {
      if (!publish(false)) {
        --s;
      } else {
        std::string message;
        m_Hello->get_string_value(message, 1);
        uint32_t index;
        m_Hello->get_uint32_value(index, 0);

        std::string storedValue;
        m_Hello->get_enum_value(storedValue, 2);
        std::cout << "Message: " << message << " with index: " << index
                  << " and ENUM: " << storedValue << " SENT" << std::endl;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
    }
  }

std::tuple<eprosima::fastrtps::types::DynamicData*, eprosima::fastrtps::types::DynamicPubSubType*>
  HelloPubSub::build_fastdds_api(eprosima::fastdds::dds::DomainParticipant* participant) {
  namespace etypes = eprosima::fastrtps::types;

  etypes::DynamicTypeBuilder_ptr created_type_uint32 =
    etypes::DynamicTypeBuilderFactory::get_instance()->create_uint32_builder();
  etypes::DynamicTypeBuilder_ptr created_type_string =
    etypes::DynamicTypeBuilderFactory::get_instance()->create_string_builder();
  etypes::DynamicTypeBuilder_ptr struct_type_builder =
    etypes::DynamicTypeBuilderFactory::get_instance()->create_struct_builder();
  etypes::DynamicTypeBuilder_ptr created_type_enum =
    etypes::DynamicTypeBuilderFactory::get_instance()->create_enum_builder();
  created_type_enum->add_empty_member(0, "ALPHA");
  created_type_enum->add_empty_member(1, "BETA");

  // Add members to the struct.
  struct_type_builder->add_member(0, "index", created_type_uint32.get());
  struct_type_builder->add_member(1, "message", created_type_string.get());
  struct_type_builder->add_member(2, "enumber", created_type_enum.get());
  struct_type_builder->apply_annotation_to_member(2, "key", "value", "true");
  struct_type_builder->set_name("HelloWorld");

  etypes::DynamicType_ptr dyn_type = struct_type_builder->build();
  auto dyn_pubsub = new etypes::DynamicPubSubType(dyn_type);
  eprosima::fastdds::dds::TypeSupport m_type(dyn_pubsub);

  //REGISTER THE TYPE
  m_type.get()->auto_fill_type_information(false);
  m_type.get()->auto_fill_type_object(false); // true
  m_type.register_type(participant);

  return std::make_tuple(etypes::DynamicDataFactory::get_instance()->create_data(dyn_type), dyn_pubsub);
}

std::tuple<eprosima::fastrtps::types::DynamicData*, eprosima::fastrtps::types::DynamicPubSubType*>
  HelloPubSub::build_xtypes_api(eprosima::fastdds::dds::DomainParticipant* participant) {
  namespace etypes = eprosima::fastrtps::types;
  namespace extypes = eprosima::xtypes;

  extypes::StructType hello("HelloWorld");
  hello.add_member(extypes::Member("index", extypes::primitive_type<uint32_t>()));
  hello.add_member(extypes::Member("message", extypes::StringType()));

  eprosima::xtypes::EnumerationType<std::uint32_t> my_enum("MyEnum");
  my_enum.add_enumerator("ALPHA");
  my_enum.add_enumerator("BETA");

  hello.add_member(extypes::Member("enumber", my_enum).key());

  etypes::DynamicTypeBuilder* builder = ddsfmu::Converter::create_builder(hello);
  etypes::DynamicType_ptr dyn_type = builder->build();
  auto dyn_pubsub = new etypes::DynamicPubSubType(dyn_type);
  eprosima::fastdds::dds::TypeSupport m_type(dyn_pubsub);

  //REGISTER THE TYPE
  m_type.get()->auto_fill_type_information(false);
  m_type.get()->auto_fill_type_object(false); // true
  m_type.register_type(participant);

  ddsfmu::Converter::register_type("HelloWorld", dyn_pubsub);
  ddsfmu::Converter::register_xtype("HelloWorld", hello);

  return std::make_tuple(etypes::DynamicDataFactory::get_instance()->create_data(dyn_type), dyn_pubsub);
}

std::tuple<eprosima::fastrtps::types::DynamicData*, eprosima::fastrtps::types::DynamicPubSubType*>
  HelloPubSub::build_xtypes_idl(eprosima::fastdds::dds::DomainParticipant* participant) {
  namespace etypes = eprosima::fastrtps::types;
  namespace extypes = eprosima::xtypes;

  std::string my_idl = R"~~~(
  enum MyEnum { ALPHA, BETA };

  struct HelloWorld
  {
    uint32 index;
    string message;
    @key MyEnum enumber;
  };
  )~~~";

  extypes::idl::Context context;
  context.preprocess = false;
  context = extypes::idl::parse(my_idl, context);

  auto hello = context.module().structure("HelloWorld");

  etypes::DynamicTypeBuilder* builder = ddsfmu::Converter::create_builder(hello);
  etypes::DynamicType_ptr dyn_type = builder->build();
  auto dyn_pubsub = new etypes::DynamicPubSubType(dyn_type);
  eprosima::fastdds::dds::TypeSupport m_type(dyn_pubsub);

  //REGISTER THE TYPE
  m_type.get()->auto_fill_type_information(false);
  m_type.get()->auto_fill_type_object(false);
  // type_object must be true for contentfilter topic, but false for complex types, otherwise seg fault
  m_type.register_type(participant);

  ddsfmu::Converter::register_type("HelloWorld", dyn_pubsub);
  ddsfmu::Converter::register_xtype("HelloWorld", hello);

  return std::make_tuple(etypes::DynamicDataFactory::get_instance()->create_data(dyn_type), dyn_pubsub);
}
