#pragma once

#include <fastdds/dds/topic/IContentFilter.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicPubSubType.h>

#include "Converter.hpp"
// properly include used stuff

namespace ddsfmu {
namespace detail {

/**
   @brief A helper to hold dynamic data type information

   A content filter gets the DynamicPubSubType, which are instantiated as both fastdds and
   xtypes DynamicData. The fastdds DynamicData is first used to serialize the candidate
   sample, and the ddsfmu::Converter creates the xtypes DynamicData. This instance is used
   to compare key member values against those provided by the user.

*/
struct FilterMemberType {
  FilterMemberType() = delete;
  FilterMemberType(
    const eprosima::fastdds::dds::TopicDataType* data_type, const std::string& type_name)
      : pubsub_type(nullptr)
      , dyn_data(nullptr)
      , key_data(ddsfmu::Converter::dynamic_data(type_name))
      , sample_data(ddsfmu::Converter::dynamic_data(type_name))
      , type_name(type_name)
      , key_count(0) {
    namespace etypes = eprosima::fastrtps::types;
    const auto* dynpubsub = dynamic_cast<const etypes::DynamicPubSubType*>(data_type);
    if (dynpubsub == nullptr) {
      throw std::runtime_error("Custom filter only works with dynamic types, was unable to cast "
                               "TopicDataType* to DynamicPubSubType* ");
    }

    eprosima::fastrtps::types::DynamicType_ptr dyn_type = dynpubsub->GetDynamicType();
    pubsub_type = new eprosima::fastrtps::types::DynamicPubSubType(dyn_type);
    dyn_data = eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(dyn_type);
  }
  eprosima::fastrtps::types::DynamicPubSubType* pubsub_type;
  eprosima::fastrtps::types::DynamicData* dyn_data;
  eprosima::xtypes::DynamicData key_data, sample_data;
  //std::vector<std::function<bool()>> comparisons;
  size_t key_count;
  std::string type_name;

  bool compare_keys() {
    bool is_equal = true;
    size_t key_a = 0;
    sample_data.for_each([&](const eprosima::xtypes::DynamicData::ReadableNode& a_node) {
      bool a_is_leaf = (a_node.type().is_primitive_type() || a_node.type().is_enumerated_type());
      bool a_is_string = a_node.type().kind() == eprosima::xtypes::TypeKind::STRING_TYPE;
      if ((a_is_leaf || a_is_string) && a_node.from_member() && a_node.from_member()->is_key()) {
        size_t key_b = 0;
        key_data.for_each([&](const eprosima::xtypes::DynamicData::ReadableNode& b_node) {
          bool b_is_leaf =
            (b_node.type().is_primitive_type() || b_node.type().is_enumerated_type());
          bool b_is_string = b_node.type().kind() == eprosima::xtypes::TypeKind::STRING_TYPE;
          if (
            (b_is_leaf || b_is_string) && b_node.from_member() && b_node.from_member()->is_key()) {
            if (key_a == key_b) { is_equal &= (a_node.data() == b_node.data()); }
            key_b++;
          }
        });
        key_a++;
      }
      if (key_a == key_count) {
        throw false; // all keys compared
      }
    });
    return is_equal;
  }

  ~FilterMemberType() {
    if (pubsub_type) delete pubsub_type;
  }
};

/**
   @brief Custom key filter for dynamic data topics
*/
class CustomKeyFilter : public eprosima::fastdds::dds::IContentFilter {
private:
  std::map<std::string, std::unique_ptr<FilterMemberType>> member_types;

public:
  /**
     @brief Construct a new CustomKeyFilter object

     @param [in] data_type A DynamicPubSubType pointer
     @param [in] type_name Dynamic data type name
     @param [in] parameters List of string parameters [Reader GUID | "|GUID UNKNOWN|", key1, .., keyN]
  */
  CustomKeyFilter(
    const eprosima::fastdds::dds::TopicDataType* data_type, const std::string& type_name,
    const eprosima::fastdds::dds::LoanableTypedCollection<const char*>& parameters) {
    if (add_type(data_type, type_name, parameters)) {
      //std::cout << "Created CustomKeyFilter for " << type_name << std::endl;
    }
  }

  /**
     @brief Check if the filter has registered reader with given GUID
     @param [in] guid Reader GUID
     @return Boolean whether it is registered or not
  */
  inline bool has_reader_GUID(const std::string& guid) {
    return static_cast<bool>(member_types.count(guid));
  }

  /**
     @brief Registers a new data type with associated dynamic data type pointer

     @param [in] data_type Dynamic data type to be registered
     @param [in] type_name Name of type to be registered
     @param [in] parameters List of string parameters [Reader GUID | "|GUID UNKNOWN|", key1, .., keyN]
  */
  bool add_type(
    const eprosima::fastdds::dds::TopicDataType* data_type, const std::string& type_name,
    const eprosima::fastdds::dds::LoanableTypedCollection<const char*>& parameters);

  virtual ~CustomKeyFilter() = default;

  /**
     @brief Evaluate filter discriminating whether the sample is relevant or not, i.e. whether it meets the filtering
     criteria

     @param [in] payload Serialized sample
     @param [in] sample_info FilterSampleInfo (unused)
     @param [in] reader_guid Reader GUID
     @return true if sample meets filter requirements. false otherwise.
  */
  bool evaluate(
    const SerializedPayload& payload, const FilterSampleInfo& sample_info,
    const GUID_t& reader_guid) const override;
};

}
}
