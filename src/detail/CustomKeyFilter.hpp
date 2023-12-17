#pragma once

#include <fastdds/dds/topic/IContentFilter.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastrtps/types/DynamicPubSubType.h>
#include <fastrtps/types/DynamicDataFactory.h>

#include "Converter.hpp"
// properly include used stuff

namespace ddsfmu {
namespace detail {

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
            if (key_a == key_b) {
              is_equal &= (a_node.data() == b_node.data());
              // Setting it once in the constructor causes segmentation fault
              /*comparisons.emplace_back([&](){
                std::cout << "Comparing ";
                std::cout << a_node.data().to_string() << " vs. ";
                std::cout << b_node.data().to_string() << std::endl;
                return a_node.data() == b_node.data(); });*/
            }
            key_b++;
          }
        });
        key_a++;
      }
      if (key_a == key_count) {
        throw false; // all keys compared
      }
    });

    /*for(auto& comp : comparisons){
      is_equal &= comp();
    }*/

    return is_equal;
  }

  ~FilterMemberType() {
    if (pubsub_type) delete pubsub_type;
  }
};

/// Custom filter class
class CustomKeyFilter : public eprosima::fastdds::dds::IContentFilter {
private:
  std::map<std::string, std::unique_ptr<FilterMemberType>> member_types;

public:
  /**
     @brief Construct a new CustomKeyFilter object

  */
  CustomKeyFilter(
    const eprosima::fastdds::dds::TopicDataType* data_type, const std::string& type_name,
    const eprosima::fastdds::dds::LoanableTypedCollection<const char*>& parameters) {
    if (add_type(data_type, type_name, parameters)) {
      //std::cout << "Created CustomKeyFilter for " << type_name << std::endl;
    }
  }

  bool has_reader_GUID(const std::string& guid) {
    return static_cast<bool>(member_types.count(guid));
  }

  bool add_type(
    const eprosima::fastdds::dds::TopicDataType* data_type, const std::string& type_name,
    const eprosima::fastdds::dds::LoanableTypedCollection<const char*>& parameters) {
    if (std::string(parameters[0]) == "|GUID UNKNOWN|") {
      //std::cout << "Skipped registering: invalid reader GUID is given" << std::endl;
      return false;
    } else {
      auto a_member = member_types.insert_or_assign(
        parameters[0], std::make_unique<FilterMemberType>(data_type, type_name));
      //std::cout << "Registered GUID: '" << parameters[0] << "'" << std::endl;

      // parameters[key_member] must be cast from std::string to member type
      std::int32_t key_member = 1;

      std::ostringstream oss;
      a_member.first->second->key_data.for_each(
        [&](eprosima::xtypes::DynamicData::WritableNode& node) {
          bool is_leaf = (node.type().is_primitive_type() || node.type().is_enumerated_type());
          bool is_string = node.type().kind() == eprosima::xtypes::TypeKind::STRING_TYPE;

          if (is_leaf || is_string) {
            if (node.from_member()) {
              oss << node.from_member()->name() << ": is key " << std::boolalpha
                  << node.from_member()->is_key() << std::endl;
            }
            if (node.from_member() && node.from_member()->is_key()) {
              if (key_member == parameters.length()) {
                throw std::runtime_error(
                  type_name + std::string(" has more @key members than parameter data provided"));
              }
              switch (node.type().kind()) {
              case eprosima::xtypes::TypeKind::BOOLEAN_TYPE: {
                bool b;
                std::istringstream(parameters[key_member++]) >> b;
                node.data() = b;
                break;
              }
              case eprosima::xtypes::TypeKind::UINT_8_TYPE:
                node.data() = static_cast<std::uint8_t>(std::stoul(parameters[key_member++]));
                break;
              case eprosima::xtypes::TypeKind::UINT_16_TYPE:
                node.data() = static_cast<std::uint16_t>(std::stoul(parameters[key_member++]));
                break;
              case eprosima::xtypes::TypeKind::UINT_32_TYPE:
                node.data() = static_cast<std::uint32_t>(std::stoul(parameters[key_member++]));
                break;
              case eprosima::xtypes::TypeKind::UINT_64_TYPE:
                node.data() = std::stoull(parameters[key_member++]);
                break;
              case eprosima::xtypes::TypeKind::INT_8_TYPE:
                node.data() = static_cast<std::int8_t>(std::stoi(parameters[key_member++]));
                break;
              case eprosima::xtypes::TypeKind::INT_16_TYPE:
                node.data() = static_cast<std::int16_t>(std::stoi(parameters[key_member++]));
                break;
              case eprosima::xtypes::TypeKind::INT_32_TYPE:
                node.data() = static_cast<std::int32_t>(std::stoi(parameters[key_member++]));
                break;
              case eprosima::xtypes::TypeKind::INT_64_TYPE:
                node.data() = static_cast<std::int64_t>(std::stoll(parameters[key_member++]));
                break;
              case eprosima::xtypes::TypeKind::FLOAT_32_TYPE:
                node.data() = std::stof(parameters[key_member++]);
                break;
              case eprosima::xtypes::TypeKind::FLOAT_64_TYPE:
                node.data() = std::stod(parameters[key_member++]);
                break;
              case eprosima::xtypes::TypeKind::STRING_TYPE:
                node.data() = parameters[key_member++];
                break;
              case eprosima::xtypes::TypeKind::CHAR_8_TYPE:
                node.data() = parameters[key_member++][0];
                break;
              case eprosima::xtypes::TypeKind::ENUMERATION_TYPE:
                node.data() = static_cast<std::uint32_t>(std::stoul(parameters[key_member++]));
                break;
              case eprosima::xtypes::TypeKind::FLOAT_128_TYPE:
              case eprosima::xtypes::TypeKind::CHAR_16_TYPE:
              case eprosima::xtypes::TypeKind::WIDE_CHAR_TYPE:
              case eprosima::xtypes::TypeKind::BITSET_TYPE:
              case eprosima::xtypes::TypeKind::ALIAS_TYPE:    // Needed?
              case eprosima::xtypes::TypeKind::SEQUENCE_TYPE: // std::vector
              case eprosima::xtypes::TypeKind::WSTRING_TYPE:
              case eprosima::xtypes::TypeKind::MAP_TYPE:
                // unsupported
              default: break;
              }
            }
          }
        });
      a_member.first->second->key_count = key_member - 1;

      //std::cout << oss.str();
    }
    return true;
  }

  virtual ~CustomKeyFilter() {}

  /**
     @brief Evaluate filter discriminating whether the sample is relevant or not, i.e. whether it meets the filtering
     criteria

     @param payload Serialized sample
     @return true if sample meets filter requirements. false otherwise.
  */
  bool evaluate(
    const SerializedPayload& payload, const FilterSampleInfo& sample_info,
    const GUID_t& reader_guid) const override {
    std::ostringstream guid;
    guid << reader_guid; // The only useful identifier for a data reader

    try {
      //std::cout << "Retrieving GUID: " << guid.str() << std::endl;
      FilterMemberType* member_type = member_types.at(guid.str()).get();
      //std::cout << "Got filter member: " << member_type->type_name << std::endl;
      SerializedPayload payload_copy(payload.length);
      if (!payload_copy.copy(&payload)) {
        std::cerr << "Could not copy serialized payload." << std::endl;
        return false;
      }
      if (!member_type->pubsub_type->deserialize(&payload_copy, member_type->dyn_data)) {
        std::cerr << "Could not deserialize payload to dynamic type" << std::endl;
        return false;
      }

      if (!member_type->pubsub_type->m_isGetKeyDefined) {
        std::cerr << "Type has not GetKeyDefined - A nested member?" << std::endl;
        // Keep sample
        return true;
      } else {
        bool ok_conversion =
          ddsfmu::Converter::fastdds_to_xtypes(member_type->dyn_data, member_type->sample_data);
        if (!ok_conversion) { return false; }
        //std::cout << member_type->sample_data << std::endl;
        return member_type->compare_keys(); // Key comparison is done here
      }

    } catch (const std::out_of_range& e) {
      // DataReader in question is not registered and thus irrelevant
    }

    return false; // return false if sample is rejected, true otherwise
  }
};

}
}
