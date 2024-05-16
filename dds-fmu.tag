<?xml version='1.0' encoding='UTF-8' standalone='yes' ?>
<tagfile doxygen_version="1.9.4">
  <compound kind="struct">
    <name>ddsfmu::Converter</name>
    <filename>structddsfmu_1_1Converter.html</filename>
    <member kind="function" static="yes">
      <type>static bool</type>
      <name>xtypes_to_fastdds</name>
      <anchorfile>structddsfmu_1_1Converter.html</anchorfile>
      <anchor>a556f3a6b5b32cef2a134023586861daa</anchor>
      <arglist>(const eprosima::xtypes::DynamicData &amp;input, eprosima::fastrtps::types::DynamicData *output)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static bool</type>
      <name>fastdds_to_xtypes</name>
      <anchorfile>structddsfmu_1_1Converter.html</anchorfile>
      <anchor>af5e1f95517ca2aa92b00f90574e16cf7</anchor>
      <arglist>(const eprosima::fastrtps::types::DynamicData *input, eprosima::xtypes::DynamicData &amp;output)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static eprosima::xtypes::DynamicData</type>
      <name>dynamic_data</name>
      <anchorfile>structddsfmu_1_1Converter.html</anchorfile>
      <anchor>a8bed1114773a62d0f8074f7ec3af8f70</anchor>
      <arglist>(const std::string &amp;type_name)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>register_type</name>
      <anchorfile>structddsfmu_1_1Converter.html</anchorfile>
      <anchor>a025917088cb85b66250d212713823db2</anchor>
      <arglist>(const std::string &amp;type_name, eprosima::fastrtps::types::DynamicPubSubType *type)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>register_xtype</name>
      <anchorfile>structddsfmu_1_1Converter.html</anchorfile>
      <anchor>a43c58351a903325d807bcbadf0054784</anchor>
      <arglist>(const std::string &amp;type_name, const eprosima::xtypes::DynamicType &amp;type)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static eprosima::fastrtps::types::DynamicTypeBuilder *</type>
      <name>create_builder</name>
      <anchorfile>structddsfmu_1_1Converter.html</anchorfile>
      <anchor>a6f460dddca508c59d683be19ce052644</anchor>
      <arglist>(const eprosima::xtypes::DynamicType &amp;type)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static std::string</type>
      <name>convert_type_name</name>
      <anchorfile>structddsfmu_1_1Converter.html</anchorfile>
      <anchor>a42cad855ffb58e1c1ab8df70e33729fe</anchor>
      <arglist>(const std::string &amp;message_type)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static const eprosima::xtypes::DynamicType &amp;</type>
      <name>resolve_discriminator_type</name>
      <anchorfile>structddsfmu_1_1Converter.html</anchorfile>
      <anchor>afdde9a0b68520ec17ab5d92aae58622d</anchor>
      <arglist>(const eprosima::xtypes::DynamicType &amp;service_type, const std::string &amp;discriminator)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static eprosima::xtypes::WritableDynamicDataRef</type>
      <name>access_member_data</name>
      <anchorfile>structddsfmu_1_1Converter.html</anchorfile>
      <anchor>a72692517809ef84d349e3b1741f39c99</anchor>
      <arglist>(eprosima::xtypes::WritableDynamicDataRef membered_data, const std::string &amp;path)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>clear_data_structures</name>
      <anchorfile>structddsfmu_1_1Converter.html</anchorfile>
      <anchor>ab062151d979041865ec9366ddaf568cd</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>ddsfmu::detail::CustomKeyFilter</name>
    <filename>classddsfmu_1_1detail_1_1CustomKeyFilter.html</filename>
    <member kind="function">
      <type></type>
      <name>CustomKeyFilter</name>
      <anchorfile>classddsfmu_1_1detail_1_1CustomKeyFilter.html</anchorfile>
      <anchor>aeb86b62128a4d056d183cb4b2ea62586</anchor>
      <arglist>(const eprosima::fastdds::dds::TopicDataType *data_type, const std::string &amp;type_name, const eprosima::fastdds::dds::LoanableTypedCollection&lt; const char * &gt; &amp;parameters)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>has_reader_GUID</name>
      <anchorfile>classddsfmu_1_1detail_1_1CustomKeyFilter.html</anchorfile>
      <anchor>a019ae8d725825cb630a60a7c413764d4</anchor>
      <arglist>(const std::string &amp;guid)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>add_type</name>
      <anchorfile>classddsfmu_1_1detail_1_1CustomKeyFilter.html</anchorfile>
      <anchor>ab8f3cdc65d6ae4c961e909805ba668b5</anchor>
      <arglist>(const eprosima::fastdds::dds::TopicDataType *data_type, const std::string &amp;type_name, const eprosima::fastdds::dds::LoanableTypedCollection&lt; const char * &gt; &amp;parameters)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual</type>
      <name>~CustomKeyFilter</name>
      <anchorfile>classddsfmu_1_1detail_1_1CustomKeyFilter.html</anchorfile>
      <anchor>a497e3051b6186bc45148b4715db02308</anchor>
      <arglist>()=default</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>evaluate</name>
      <anchorfile>classddsfmu_1_1detail_1_1CustomKeyFilter.html</anchorfile>
      <anchor>a80da85cd504224601062937bede71f5b</anchor>
      <arglist>(const SerializedPayload &amp;payload, const FilterSampleInfo &amp;sample_info, const GUID_t &amp;reader_guid) const override</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>ddsfmu::detail::CustomKeyFilterFactory</name>
    <filename>classddsfmu_1_1detail_1_1CustomKeyFilterFactory.html</filename>
    <member kind="function">
      <type>eprosima::fastrtps::types::ReturnCode_t</type>
      <name>create_content_filter</name>
      <anchorfile>classddsfmu_1_1detail_1_1CustomKeyFilterFactory.html</anchorfile>
      <anchor>a023c5bcfb6a044ec247461fd20302770</anchor>
      <arglist>(const char *filter_class_name, const char *type_name, const eprosima::fastdds::dds::TopicDataType *data_type, const char *, const ParameterSeq &amp;filter_parameters, eprosima::fastdds::dds::IContentFilter *&amp;filter_instance) override</arglist>
    </member>
    <member kind="function">
      <type>eprosima::fastrtps::types::ReturnCode_t</type>
      <name>delete_content_filter</name>
      <anchorfile>classddsfmu_1_1detail_1_1CustomKeyFilterFactory.html</anchorfile>
      <anchor>acd2bcf591d9d65a2d7ee2c42e3248234</anchor>
      <arglist>(const char *filter_class_name, eprosima::fastdds::dds::IContentFilter *filter_instance) override</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>ddsfmu::DataMapper</name>
    <filename>classddsfmu_1_1DataMapper.html</filename>
    <member kind="enumeration">
      <type></type>
      <name>Direction</name>
      <anchorfile>classddsfmu_1_1DataMapper.html</anchorfile>
      <anchor>a96685be3bca70cb3b661808f28e66845</anchor>
      <arglist></arglist>
      <enumvalue file="classddsfmu_1_1DataMapper.html" anchor="a96685be3bca70cb3b661808f28e66845a7a1a5f3e79fdc91edf2f5ead9d66abb4">Read</enumvalue>
      <enumvalue file="classddsfmu_1_1DataMapper.html" anchor="a96685be3bca70cb3b661808f28e66845a1129c0e4d43f2d121652a7302712cff6">Write</enumvalue>
      <enumvalue file="classddsfmu_1_1DataMapper.html" anchor="a96685be3bca70cb3b661808f28e66845a83f499a540b1323009c200d6f8cc9396">Parameter</enumvalue>
    </member>
    <member kind="typedef">
      <type>std::tuple&lt; std::int32_t, std::int32_t, std::int32_t, std::int32_t &gt;</type>
      <name>IndexOffsets</name>
      <anchorfile>classddsfmu_1_1DataMapper.html</anchorfile>
      <anchor>a730d0504e0fcf2509954e3b05e1b1eb3</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>DataMapper</name>
      <anchorfile>classddsfmu_1_1DataMapper.html</anchorfile>
      <anchor>a0c465cc99917d87627b2044eb0d48a98</anchor>
      <arglist>()=default</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>DataMapper</name>
      <anchorfile>classddsfmu_1_1DataMapper.html</anchorfile>
      <anchor>aa70ae7223be0ee93714464bcd4ae97fd</anchor>
      <arglist>(const DataMapper &amp;)=delete</arglist>
    </member>
    <member kind="function">
      <type>DataMapper &amp;</type>
      <name>operator=</name>
      <anchorfile>classddsfmu_1_1DataMapper.html</anchorfile>
      <anchor>a24842b5a803d1f71457bcfb9474e8838</anchor>
      <arglist>(const DataMapper &amp;)=delete</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>reset</name>
      <anchorfile>classddsfmu_1_1DataMapper.html</anchorfile>
      <anchor>a8b61d7ee755629f2528e0588c65ba955</anchor>
      <arglist>(const std::filesystem::path &amp;fmu_resources)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set_double</name>
      <anchorfile>classddsfmu_1_1DataMapper.html</anchorfile>
      <anchor>ad7d8b4ced7070c0a82aca988d6a9492e</anchor>
      <arglist>(const std::int32_t value_ref, const double &amp;value)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>get_double</name>
      <anchorfile>classddsfmu_1_1DataMapper.html</anchorfile>
      <anchor>a1981e8721932b373980e9f56d882ddbd</anchor>
      <arglist>(const std::int32_t value_ref, double &amp;value) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set_int</name>
      <anchorfile>classddsfmu_1_1DataMapper.html</anchorfile>
      <anchor>a14b603dc232576248902dba6e5aff7de</anchor>
      <arglist>(const std::int32_t value_ref, const std::int32_t &amp;value)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>get_int</name>
      <anchorfile>classddsfmu_1_1DataMapper.html</anchorfile>
      <anchor>a34e9048d89dc4238685fff3d2565f66e</anchor>
      <arglist>(const std::int32_t value_ref, std::int32_t &amp;value) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set_bool</name>
      <anchorfile>classddsfmu_1_1DataMapper.html</anchorfile>
      <anchor>a164af7d68ef96e4e85fb4dae50b7aaab</anchor>
      <arglist>(const std::int32_t value_ref, const bool &amp;value)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>get_bool</name>
      <anchorfile>classddsfmu_1_1DataMapper.html</anchorfile>
      <anchor>a10193dacbe55953490f41a9bed82634f</anchor>
      <arglist>(const std::int32_t value_ref, bool &amp;value) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set_string</name>
      <anchorfile>classddsfmu_1_1DataMapper.html</anchorfile>
      <anchor>afc8e74c5501cd8a79d38f0eaccace817</anchor>
      <arglist>(const std::int32_t value_ref, const std::string &amp;value)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>get_string</name>
      <anchorfile>classddsfmu_1_1DataMapper.html</anchorfile>
      <anchor>a375d2b338019d70c8b5b3bc230ac21dd</anchor>
      <arglist>(const std::int32_t value_ref, std::string &amp;value) const</arglist>
    </member>
    <member kind="function">
      <type>eprosima::xtypes::DynamicData &amp;</type>
      <name>data_ref</name>
      <anchorfile>classddsfmu_1_1DataMapper.html</anchorfile>
      <anchor>ab2cadb8f9ac49885fb6e8b086365ec18</anchor>
      <arglist>(const std::string &amp;topic, Direction read_write_param)</arglist>
    </member>
    <member kind="function">
      <type>const eprosima::xtypes::DynamicData &amp;</type>
      <name>data_ref</name>
      <anchorfile>classddsfmu_1_1DataMapper.html</anchorfile>
      <anchor>a5553ae90632de14d007fe264196697c3</anchor>
      <arglist>(const std::string &amp;topic, Direction read_write_param) const</arglist>
    </member>
    <member kind="function">
      <type>eprosima::xtypes::idl::Context &amp;</type>
      <name>idl_context</name>
      <anchorfile>classddsfmu_1_1DataMapper.html</anchorfile>
      <anchor>aa562f948a97d654e80836782fa02c116</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>IndexOffsets</type>
      <name>index_offsets</name>
      <anchorfile>classddsfmu_1_1DataMapper.html</anchorfile>
      <anchor>acec66794b7ea077bfc891266b9983115</anchor>
      <arglist>(const std::string &amp;topic, Direction read_write_param) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>queue_for_key_parameter</name>
      <anchorfile>classddsfmu_1_1DataMapper.html</anchorfile>
      <anchor>a907a606415d9228e40a26c9a380245e6</anchor>
      <arglist>(const std::string &amp;topic_name, const std::string &amp;topic_type)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>process_key_queue</name>
      <anchorfile>classddsfmu_1_1DataMapper.html</anchorfile>
      <anchor>ae36b1efa1c3b5ba7d82aab2fb68dacdf</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>ddsfmu::DomainListener</name>
    <filename>classddsfmu_1_1DomainListener.html</filename>
    <member kind="function">
      <type></type>
      <name>DomainListener</name>
      <anchorfile>classddsfmu_1_1DomainListener.html</anchorfile>
      <anchor>aa2bd753d04057130f01de3c092f50d22</anchor>
      <arglist>()=default</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>~DomainListener</name>
      <anchorfile>classddsfmu_1_1DomainListener.html</anchorfile>
      <anchor>abf367b60f600a5c468e496575462e6a0</anchor>
      <arglist>() override=default</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>on_requested_incompatible_qos</name>
      <anchorfile>classddsfmu_1_1DomainListener.html</anchorfile>
      <anchor>af216955d7c44d1b207f5c061a8d1394d</anchor>
      <arglist>(eprosima::fastdds::dds::DataReader *reader, const eprosima::fastdds::dds::RequestedIncompatibleQosStatus &amp;status) override</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>on_offered_incompatible_qos</name>
      <anchorfile>classddsfmu_1_1DomainListener.html</anchorfile>
      <anchor>a88a97845d61f4f71d360c88dab80246f</anchor>
      <arglist>(eprosima::fastdds::dds::DataWriter *writer, const eprosima::fastdds::dds::OfferedIncompatibleQosStatus &amp;status) override</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>on_inconsistent_topic</name>
      <anchorfile>classddsfmu_1_1DomainListener.html</anchorfile>
      <anchor>a8a4b556bbc0a59cb847bdc49a47e40b7</anchor>
      <arglist>(eprosima::fastdds::dds::Topic *topic, eprosima::fastdds::dds::InconsistentTopicStatus status) override</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>ddsfmu::DynamicPubSub</name>
    <filename>classddsfmu_1_1DynamicPubSub.html</filename>
    <member kind="function">
      <type></type>
      <name>DynamicPubSub</name>
      <anchorfile>classddsfmu_1_1DynamicPubSub.html</anchorfile>
      <anchor>a1e4fa81125aa8529936f57ade9a830e0</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>~DynamicPubSub</name>
      <anchorfile>classddsfmu_1_1DynamicPubSub.html</anchorfile>
      <anchor>abb95e297beec62fab8da5fecea0ae14a</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>DynamicPubSub</name>
      <anchorfile>classddsfmu_1_1DynamicPubSub.html</anchorfile>
      <anchor>a721ee9b2db1c9b124d6932aac35a103b</anchor>
      <arglist>(const DynamicPubSub &amp;)=delete</arglist>
    </member>
    <member kind="function">
      <type>DynamicPubSub &amp;</type>
      <name>operator=</name>
      <anchorfile>classddsfmu_1_1DynamicPubSub.html</anchorfile>
      <anchor>aed4cb3208b9a70b796261bd6afbc3768</anchor>
      <arglist>(const DynamicPubSub &amp;)=delete</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>reset</name>
      <anchorfile>classddsfmu_1_1DynamicPubSub.html</anchorfile>
      <anchor>afe2c6990b34515b9e924f4fee46ffb13</anchor>
      <arglist>(const std::filesystem::path &amp;fmu_resources, DataMapper *const mapper, const std::string &amp;name=&quot;dds-fmu&quot;, cppfmu::Logger *const logger=nullptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>write</name>
      <anchorfile>classddsfmu_1_1DynamicPubSub.html</anchorfile>
      <anchor>accaec41f1b65eceded3a3202d5103a7a</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>take</name>
      <anchorfile>classddsfmu_1_1DynamicPubSub.html</anchorfile>
      <anchor>a2bb4e8c0f64f01e95a479d455ff7ea68</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>init_key_filters</name>
      <anchorfile>classddsfmu_1_1DynamicPubSub.html</anchorfile>
      <anchor>aab463e6d7a0694257b98e04dc56d0672</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>ddsfmu::detail::FilterMemberType</name>
    <filename>structddsfmu_1_1detail_1_1FilterMemberType.html</filename>
    <member kind="function">
      <type></type>
      <name>FilterMemberType</name>
      <anchorfile>structddsfmu_1_1detail_1_1FilterMemberType.html</anchorfile>
      <anchor>ac8c2c32cc8d9d12f103b669a89234863</anchor>
      <arglist>()=delete</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>FilterMemberType</name>
      <anchorfile>structddsfmu_1_1detail_1_1FilterMemberType.html</anchorfile>
      <anchor>a059a114e6c092ff2577d97c460624f75</anchor>
      <arglist>(const eprosima::fastdds::dds::TopicDataType *data_type, const std::string &amp;type_name)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>compare_keys</name>
      <anchorfile>structddsfmu_1_1detail_1_1FilterMemberType.html</anchorfile>
      <anchor>aca15310c0637ac26e80589de7687ed66</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>~FilterMemberType</name>
      <anchorfile>structddsfmu_1_1detail_1_1FilterMemberType.html</anchorfile>
      <anchor>a01b5c6677487a7894fb610ffa0bf8103</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="variable">
      <type>eprosima::fastrtps::types::DynamicPubSubType *</type>
      <name>pubsub_type</name>
      <anchorfile>structddsfmu_1_1detail_1_1FilterMemberType.html</anchorfile>
      <anchor>aa67950787a52a9bacf63515d9b3078e7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>eprosima::fastrtps::types::DynamicData *</type>
      <name>dyn_data</name>
      <anchorfile>structddsfmu_1_1detail_1_1FilterMemberType.html</anchorfile>
      <anchor>aaae8e2f4c0d9cb538880ded12271b141</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>eprosima::xtypes::DynamicData</type>
      <name>key_data</name>
      <anchorfile>structddsfmu_1_1detail_1_1FilterMemberType.html</anchorfile>
      <anchor>ac38fc650146cb69d1b57394b88e6b220</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>eprosima::xtypes::DynamicData</type>
      <name>sample_data</name>
      <anchorfile>structddsfmu_1_1detail_1_1FilterMemberType.html</anchorfile>
      <anchor>a0165bd59d35c9ea2e6f30532d9a48ec4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>size_t</type>
      <name>key_count</name>
      <anchorfile>structddsfmu_1_1detail_1_1FilterMemberType.html</anchorfile>
      <anchor>aca293a32a936eb2df5b715221604b9ef</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>std::string</type>
      <name>type_name</name>
      <anchorfile>structddsfmu_1_1detail_1_1FilterMemberType.html</anchorfile>
      <anchor>a9d85a49ce72f5ded7d905827ead52807</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>ddsfmu::FmiLogger</name>
    <filename>classddsfmu_1_1FmiLogger.html</filename>
    <member kind="function">
      <type></type>
      <name>FmiLogger</name>
      <anchorfile>classddsfmu_1_1FmiLogger.html</anchorfile>
      <anchor>aa18ab2abd3762c7ea453cea634f0a455</anchor>
      <arglist>()=delete</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>FmiLogger</name>
      <anchorfile>classddsfmu_1_1FmiLogger.html</anchorfile>
      <anchor>a872593468d30059595a523c81c2a8744</anchor>
      <arglist>(cppfmu::Logger &amp;logger, const std::string &amp;name)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual</type>
      <name>~FmiLogger</name>
      <anchorfile>classddsfmu_1_1FmiLogger.html</anchorfile>
      <anchor>a3c92e5f7527298f1beee1fd11736ad9a</anchor>
      <arglist>()=default</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Consume</name>
      <anchorfile>classddsfmu_1_1FmiLogger.html</anchorfile>
      <anchor>a655f9ddcf2903eaaaece694603bcacec</anchor>
      <arglist>(const eprosima::fastdds::dds::Log::Entry &amp;entry) override</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>std::ostream &amp;</type>
      <name>get_stream</name>
      <anchorfile>classddsfmu_1_1FmiLogger.html</anchorfile>
      <anchor>a69cb2c7529f6d838574a3f5c3760fcb6</anchor>
      <arglist>(const eprosima::fastdds::dds::Log::Entry &amp;entry) override</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>ddsfmu::FmuInstance</name>
    <filename>classddsfmu_1_1FmuInstance.html</filename>
    <member kind="function">
      <type></type>
      <name>FmuInstance</name>
      <anchorfile>classddsfmu_1_1FmuInstance.html</anchorfile>
      <anchor>ac9edc9afca21378e7f39106a0502370a</anchor>
      <arglist>(const std::string &amp;name, const std::filesystem::path &amp;resource_path, cppfmu::Logger logger)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetReal</name>
      <anchorfile>classddsfmu_1_1FmuInstance.html</anchorfile>
      <anchor>aaac42ab0c35520e8403809ae7fae8dc8</anchor>
      <arglist>(const cppfmu::FMIValueReference vr[], std::size_t nvr, const cppfmu::FMIReal value[]) override</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>GetReal</name>
      <anchorfile>classddsfmu_1_1FmuInstance.html</anchorfile>
      <anchor>aad75b41f821bd63d2eaca83a74e387c5</anchor>
      <arglist>(const cppfmu::FMIValueReference vr[], std::size_t nvr, cppfmu::FMIReal value[]) const override</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetInteger</name>
      <anchorfile>classddsfmu_1_1FmuInstance.html</anchorfile>
      <anchor>ad23c0cff139942d6937ab3d9c4f810b4</anchor>
      <arglist>(const cppfmu::FMIValueReference vr[], std::size_t nvr, const cppfmu::FMIInteger value[]) override</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>GetInteger</name>
      <anchorfile>classddsfmu_1_1FmuInstance.html</anchorfile>
      <anchor>a48712cde8db0900b9b461a5bc6d514f7</anchor>
      <arglist>(const cppfmu::FMIValueReference vr[], std::size_t nvr, cppfmu::FMIInteger value[]) const override</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetBoolean</name>
      <anchorfile>classddsfmu_1_1FmuInstance.html</anchorfile>
      <anchor>ad03a79b4d9d8bfcd7b66be9fd4f155cb</anchor>
      <arglist>(const cppfmu::FMIValueReference vr[], std::size_t nvr, const cppfmu::FMIBoolean value[]) override</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>GetBoolean</name>
      <anchorfile>classddsfmu_1_1FmuInstance.html</anchorfile>
      <anchor>ab72e43c632b3b2f768957951238ebfc4</anchor>
      <arglist>(const cppfmu::FMIValueReference vr[], std::size_t nvr, cppfmu::FMIBoolean value[]) const override</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetString</name>
      <anchorfile>classddsfmu_1_1FmuInstance.html</anchorfile>
      <anchor>a7c5bda4cbc1f9d52234d9bb167404dc3</anchor>
      <arglist>(const cppfmu::FMIValueReference vr[], std::size_t nvr, const cppfmu::FMIString value[]) override</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>GetString</name>
      <anchorfile>classddsfmu_1_1FmuInstance.html</anchorfile>
      <anchor>a28409aa8716fd8237ba6d77c8eab13bf</anchor>
      <arglist>(const cppfmu::FMIValueReference vr[], std::size_t nvr, cppfmu::FMIString value[]) const override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>ExitInitializationMode</name>
      <anchorfile>classddsfmu_1_1FmuInstance.html</anchorfile>
      <anchor>ad37b12476f15e8d0eec939c35b111a9f</anchor>
      <arglist>() override</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>~FmuInstance</name>
      <anchorfile>classddsfmu_1_1FmuInstance.html</anchorfile>
      <anchor>aad47cab23b4432e72879cfb82ea03d87</anchor>
      <arglist>()=default</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetupExperiment</name>
      <anchorfile>classddsfmu_1_1FmuInstance.html</anchorfile>
      <anchor>a4cdf11b33e0b5ae16369ed41ac158fae</anchor>
      <arglist>(cppfmu::FMIBoolean, cppfmu::FMIReal, cppfmu::FMIReal tStart, cppfmu::FMIBoolean, cppfmu::FMIReal) override</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>DoStep</name>
      <anchorfile>classddsfmu_1_1FmuInstance.html</anchorfile>
      <anchor>a6241f2b1ef569d34283cc005b692ed2d</anchor>
      <arglist>(cppfmu::FMIReal currentCommunicationPoint, cppfmu::FMIReal communicationStepSize, cppfmu::FMIBoolean, cppfmu::FMIReal &amp;) override</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Reset</name>
      <anchorfile>classddsfmu_1_1FmuInstance.html</anchorfile>
      <anchor>afd714f7deef92914dd8e56fa9051f78c</anchor>
      <arglist>() override</arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>idl::Info</name>
    <filename>structidl_1_1Info.html</filename>
    <member kind="variable">
      <type>double</type>
      <name>number</name>
      <anchorfile>structidl_1_1Info.html</anchorfile>
      <anchor>a11a769393a11520ee01dc441a0746e1c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>boolean</type>
      <name>active</name>
      <anchorfile>structidl_1_1Info.html</anchorfile>
      <anchor>a9a35e7538ae06de1a7389af600a46305</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>key uint32</type>
      <name>id</name>
      <anchorfile>structidl_1_1Info.html</anchorfile>
      <anchor>a20c4f3e986f2d6c679cee6e6c7b12f8a</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>idl::Signal</name>
    <filename>structidl_1_1Signal.html</filename>
    <member kind="variable">
      <type>double</type>
      <name>value</name>
      <anchorfile>structidl_1_1Signal.html</anchorfile>
      <anchor>a8ee8a7b1eb2d2c7ff05940fc8119e51f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>key uint16</type>
      <name>my_key</name>
      <anchorfile>structidl_1_1Signal.html</anchorfile>
      <anchor>a3b4f173e503d41552a8b72df12f83761</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>boolean</type>
      <name>is_pos</name>
      <anchorfile>structidl_1_1Signal.html</anchorfile>
      <anchor>a5bf93cc8296354f5605e7a7909c23fe6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>MyIndex</type>
      <name>my_enum</name>
      <anchorfile>structidl_1_1Signal.html</anchorfile>
      <anchor>a719edebf005e668c90e413872b9b2d75</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>ddsfmu::SignalDistributor</name>
    <filename>classddsfmu_1_1SignalDistributor.html</filename>
    <member kind="enumeration">
      <type></type>
      <name>Cardinality</name>
      <anchorfile>classddsfmu_1_1SignalDistributor.html</anchorfile>
      <anchor>a92b5a6cb0fca2419cbf45181f6556623</anchor>
      <arglist></arglist>
      <enumvalue file="classddsfmu_1_1SignalDistributor.html" anchor="a92b5a6cb0fca2419cbf45181f6556623aa84cc046d48610b05c21fd3670d0c829">INPUT</enumvalue>
      <enumvalue file="classddsfmu_1_1SignalDistributor.html" anchor="a92b5a6cb0fca2419cbf45181f6556623a50a87f0d71f7221582dad4bf507a0f34">OUTPUT</enumvalue>
      <enumvalue file="classddsfmu_1_1SignalDistributor.html" anchor="a92b5a6cb0fca2419cbf45181f6556623abf1d4bbf6ed8626603df4a7cac94ee8c">PARAMETER</enumvalue>
    </member>
    <member kind="function">
      <type></type>
      <name>SignalDistributor</name>
      <anchorfile>classddsfmu_1_1SignalDistributor.html</anchorfile>
      <anchor>a3c6b5016c420e97ad8ea04c9938a321b</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>load_idls</name>
      <anchorfile>classddsfmu_1_1SignalDistributor.html</anchorfile>
      <anchor>a55f3da90233080eb8429c28db44a2b8a</anchor>
      <arglist>(const std::filesystem::path &amp;resource_path)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>has_structure</name>
      <anchorfile>classddsfmu_1_1SignalDistributor.html</anchorfile>
      <anchor>a1b60363934fda51512a92672647ff2dd</anchor>
      <arglist>(const std::string &amp;topic_type)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>add</name>
      <anchorfile>classddsfmu_1_1SignalDistributor.html</anchorfile>
      <anchor>a83147ed1782122ea7ad89ba290cba789</anchor>
      <arglist>(const std::string &amp;topic_name, const std::string &amp;topic_type, Cardinality cardinal)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>queue_for_key_parameter</name>
      <anchorfile>classddsfmu_1_1SignalDistributor.html</anchorfile>
      <anchor>a435da4f83ea1fd2bc4e4e16965a52308</anchor>
      <arglist>(const std::string &amp;topic_name, const std::string &amp;topic_type)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>process_key_queue</name>
      <anchorfile>classddsfmu_1_1SignalDistributor.html</anchorfile>
      <anchor>abfc27dd2dc8187714c5ecd3eb1e720d2</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const std::vector&lt; SignalInfo &gt; &amp;</type>
      <name>get_mapping</name>
      <anchorfile>classddsfmu_1_1SignalDistributor.html</anchorfile>
      <anchor>a62bb00344d036f8d11b509c0353783ae</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>std::uint32_t</type>
      <name>outputs</name>
      <anchorfile>classddsfmu_1_1SignalDistributor.html</anchorfile>
      <anchor>ae81f3ee7abdaa1c424363da2593f455f</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static config::ScalarVariableType</type>
      <name>resolve_type</name>
      <anchorfile>classddsfmu_1_1SignalDistributor.html</anchorfile>
      <anchor>acb03b9cf5639ed090ccad3e58c52b728</anchor>
      <arglist>(const eprosima::xtypes::DynamicData::ReadableNode &amp;node)</arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>idl::TestData</name>
    <filename>structidl_1_1TestData.html</filename>
    <member kind="variable">
      <type>string</type>
      <name>message</name>
      <anchorfile>structidl_1_1TestData.html</anchorfile>
      <anchor>a94d1c0b086bc2fe717d4482ff24f4771</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32</type>
      <name>index</name>
      <anchorfile>structidl_1_1TestData.html</anchorfile>
      <anchor>a74625021d7d54f28a56ef457cef2cf9c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32</type>
      <name>my_matrix</name>
      <anchorfile>structidl_1_1TestData.html</anchorfile>
      <anchor>a77205262f982d31f299a16f4b8ca9ff6</anchor>
      <arglist>[5][2]</arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>idl::Trigonometry</name>
    <filename>structidl_1_1Trigonometry.html</filename>
    <member kind="variable">
      <type>double</type>
      <name>sine</name>
      <anchorfile>structidl_1_1Trigonometry.html</anchorfile>
      <anchor>a90b97bc0aacff8c68e90f229eef57bce</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>cosine</name>
      <anchorfile>structidl_1_1Trigonometry.html</anchorfile>
      <anchor>a8587de416b3ba0672ee0a489af8e6f7b</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="namespace">
    <name>cppfmu</name>
    <filename>namespacecppfmu.html</filename>
  </compound>
  <compound kind="namespace">
    <name>ddsfmu</name>
    <filename>namespaceddsfmu.html</filename>
    <namespace>ddsfmu::config</namespace>
    <namespace>ddsfmu::detail</namespace>
    <class kind="struct">ddsfmu::Converter</class>
    <class kind="class">ddsfmu::DataMapper</class>
    <class kind="class">ddsfmu::DomainListener</class>
    <class kind="class">ddsfmu::DynamicPubSub</class>
    <class kind="class">ddsfmu::FmiLogger</class>
    <class kind="class">ddsfmu::FmuInstance</class>
    <class kind="class">ddsfmu::SignalDistributor</class>
    <member kind="typedef">
      <type>std::tuple&lt; std::uint32_t, std::string, std::string, config::ScalarVariableType &gt;</type>
      <name>SignalInfo</name>
      <anchorfile>namespaceddsfmu.html</anchorfile>
      <anchor>ac890a289297045e6be08ef599742b8f5</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="namespace">
    <name>ddsfmu::config</name>
    <filename>namespaceddsfmu_1_1config.html</filename>
    <member kind="enumeration">
      <type></type>
      <name>ScalarVariableType</name>
      <anchorfile>namespaceddsfmu_1_1config.html</anchorfile>
      <anchor>a6ed368787743de15776bf05d333d9267</anchor>
      <arglist></arglist>
      <enumvalue file="namespaceddsfmu_1_1config.html" anchor="a6ed368787743de15776bf05d333d9267a7f80fcc452c2f1ed2bb51b39d0864df1">Real</enumvalue>
      <enumvalue file="namespaceddsfmu_1_1config.html" anchor="a6ed368787743de15776bf05d333d9267aa0faef0851b4294c06f2b94bb1cb2044">Integer</enumvalue>
      <enumvalue file="namespaceddsfmu_1_1config.html" anchor="a6ed368787743de15776bf05d333d9267a27226c864bac7454a8504f8edb15d95b">Boolean</enumvalue>
      <enumvalue file="namespaceddsfmu_1_1config.html" anchor="a6ed368787743de15776bf05d333d9267a27118326006d3829667a400ad23d5d98">String</enumvalue>
      <enumvalue file="namespaceddsfmu_1_1config.html" anchor="a6ed368787743de15776bf05d333d9267a88183b946cc5f0e8c96b2e66e1c74a7e">Unknown</enumvalue>
    </member>
    <member kind="function">
      <type>std::string</type>
      <name>generate_uuid</name>
      <anchorfile>namespaceddsfmu_1_1config.html</anchorfile>
      <anchor>ac8d8a1db89b2aba3b7beb312566e0fb2</anchor>
      <arglist>(const std::vector&lt; std::filesystem::path &gt; &amp;uuid_files, const std::vector&lt; std::string &gt; &amp;strings=std::vector&lt; std::string &gt;())</arglist>
    </member>
    <member kind="function">
      <type>std::vector&lt; std::filesystem::path &gt;</type>
      <name>get_uuid_files</name>
      <anchorfile>namespaceddsfmu_1_1config.html</anchorfile>
      <anchor>aba20d4d4ee16272ed5dc6c17a19afaef</anchor>
      <arglist>(const std::filesystem::path &amp;fmu_root, bool skip_modelDescription=true)</arglist>
    </member>
    <member kind="function">
      <type>eprosima::xtypes::idl::Context</type>
      <name>load_fmu_idls</name>
      <anchorfile>namespaceddsfmu_1_1config.html</anchorfile>
      <anchor>a0a2a2e583ed2faec2604c85c2bd3d64d</anchor>
      <arglist>(const std::filesystem::path &amp;resource_path, bool print=false, const std::string &amp;main_idl=&quot;dds-fmu.idl&quot;)</arglist>
    </member>
    <member kind="function">
      <type>std::string</type>
      <name>print_xml</name>
      <anchorfile>namespaceddsfmu_1_1config.html</anchorfile>
      <anchor>a85ee121ca2588ea8f71ce80dca262134</anchor>
      <arglist>(const rapidxml::xml_document&lt;&gt; &amp;doc)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>load_template_xml</name>
      <anchorfile>namespaceddsfmu_1_1config.html</anchorfile>
      <anchor>a7d56bd23ce29d9828401bf3568d3d1c8</anchor>
      <arglist>(rapidxml::xml_document&lt;&gt; &amp;doc, const std::filesystem::path &amp;template_xml, std::vector&lt; char &gt; &amp;buffer)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>load_ddsfmu_mapping</name>
      <anchorfile>namespaceddsfmu_1_1config.html</anchorfile>
      <anchor>aaddd2ce6583dfd45467d589393d9902b</anchor>
      <arglist>(rapidxml::xml_document&lt;&gt; &amp;doc, const std::filesystem::path &amp;ddsfmu_mapping, std::vector&lt; char &gt; &amp;buffer)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>write_model_description</name>
      <anchorfile>namespaceddsfmu_1_1config.html</anchorfile>
      <anchor>a44567e02cb34e263e40af7232ea54629</anchor>
      <arglist>(const rapidxml::xml_document&lt;&gt; &amp;doc, const std::filesystem::path &amp;fmu_root)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>name_generator</name>
      <anchorfile>namespaceddsfmu_1_1config.html</anchorfile>
      <anchor>a8a157edf08978363f06b50c8fd947680</anchor>
      <arglist>(std::string &amp;name, const eprosima::xtypes::DynamicData::ReadableNode &amp;rnode)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>model_variable_generator</name>
      <anchorfile>namespaceddsfmu_1_1config.html</anchorfile>
      <anchor>a8246a724c8dbf5137f02ce02f70d9934</anchor>
      <arglist>(rapidxml::xml_document&lt;&gt; &amp;doc, rapidxml::xml_node&lt;&gt; *model_variables_node, const std::string &amp;name, const std::string &amp;causality, const std::uint32_t &amp;value_ref, const ddsfmu::config::ScalarVariableType &amp;type)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>model_structure_outputs_generator</name>
      <anchorfile>namespaceddsfmu_1_1config.html</anchorfile>
      <anchor>a6aebc69b20b3e38dc2d4439c775560e8</anchor>
      <arglist>(rapidxml::xml_document&lt;&gt; &amp;doc, rapidxml::xml_node&lt;&gt; *root, const std::uint32_t &amp;num_outputs)</arglist>
    </member>
  </compound>
  <compound kind="namespace">
    <name>ddsfmu::detail</name>
    <filename>namespaceddsfmu_1_1detail.html</filename>
    <class kind="class">ddsfmu::detail::CustomKeyFilter</class>
    <class kind="class">ddsfmu::detail::CustomKeyFilterFactory</class>
    <class kind="struct">ddsfmu::detail::FilterMemberType</class>
    <member kind="function">
      <type>void</type>
      <name>reader_visitor</name>
      <anchorfile>namespaceddsfmu_1_1detail.html</anchorfile>
      <anchor>af69795ff05ce2aa9c4896ad5c9e477b2</anchor>
      <arglist>(OutType &amp;out, const eprosima::xtypes::ReadableDynamicDataRef &amp;cref)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>writer_visitor</name>
      <anchorfile>namespaceddsfmu_1_1detail.html</anchorfile>
      <anchor>af20208cad34443988e651cfac4d79761</anchor>
      <arglist>(const InType &amp;in, eprosima::xtypes::WritableDynamicDataRef &amp;ref)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>reader_visitor&lt; std::string, char &gt;</name>
      <anchorfile>namespaceddsfmu_1_1detail.html</anchorfile>
      <anchor>af38f6d8d725a9730e7d4b7d689b472a8</anchor>
      <arglist>(std::string &amp;out, const eprosima::xtypes::ReadableDynamicDataRef &amp;cref)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>writer_visitor&lt; std::string, char &gt;</name>
      <anchorfile>namespaceddsfmu_1_1detail.html</anchorfile>
      <anchor>a8a3d3deac8b8b29ec3c123c2b2cb4bfd</anchor>
      <arglist>(const std::string &amp;in, eprosima::xtypes::WritableDynamicDataRef &amp;ref)</arglist>
    </member>
  </compound>
  <compound kind="namespace">
    <name>idl</name>
    <filename>namespaceidl.html</filename>
    <class kind="struct">idl::Info</class>
    <class kind="struct">idl::Signal</class>
    <class kind="struct">idl::TestData</class>
    <class kind="struct">idl::Trigonometry</class>
    <member kind="enumeration">
      <type></type>
      <name>MyIndex</name>
      <anchorfile>namespaceidl.html</anchorfile>
      <anchor>a8ae0ecae55f47c70cd63c1602271b7cf</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FIRST</name>
      <anchorfile>namespaceidl.html</anchorfile>
      <anchor>a8ae0ecae55f47c70cd63c1602271b7cfa773cda23a6c2d90631684f27ee98f8a5</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>SECOND</name>
      <anchorfile>namespaceidl.html</anchorfile>
      <anchor>a8ae0ecae55f47c70cd63c1602271b7cfa0a5f8b2610a448bfbf83b4b835f76462</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="page">
    <name>sec_licenses</name>
    <title>Licenses</title>
    <filename>sec_licenses.html</filename>
    <docanchor file="sec_licenses.html">md__home_runner_work_dds_fmu_dds_fmu_build_Release_gen_md_licenses</docanchor>
  </compound>
  <compound kind="page">
    <name>index</name>
    <title>Introduction</title>
    <filename>index.html</filename>
    <docanchor file="index.html">md_index</docanchor>
    <docanchor file="index.html" title="Getting started with the FMU binary">sec_gettingstarted</docanchor>
    <docanchor file="index.html" title="Usage">sec_quickstart</docanchor>
    <docanchor file="index.html" title="Configuration">sec_configuration</docanchor>
    <docanchor file="index.html" title="Data type specification with IDL">sec_idl</docanchor>
    <docanchor file="index.html" title="DDS-to-FMU mapping">sec_ddsfmu</docanchor>
    <docanchor file="index.html" title="Fast-DDS XML profiles">sec_profiles</docanchor>
  </compound>
</tagfile>
