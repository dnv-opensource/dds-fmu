#include "CustomKeyFilter.hpp"

namespace ddsfmu {
namespace detail {

  bool CustomKeyFilter::add_type(
    const eprosima::fastdds::dds::TopicDataType* data_type, const std::string& type_name,
    const eprosima::fastdds::dds::LoanableTypedCollection<const char*>& parameters) {
    if (std::string(parameters[0]) == "|GUID UNKNOWN|") {
      return false;
    } else {
      auto a_member = member_types.insert_or_assign(
        parameters[0], std::make_unique<FilterMemberType>(data_type, type_name));

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

  bool CustomKeyFilter::evaluate(
    const SerializedPayload& payload, const FilterSampleInfo&,
    const GUID_t& reader_guid) const {
    std::ostringstream guid;
    guid << reader_guid; // The only useful identifier for a data reader

    try {
      //std::cout << "Retrieving GUID: " << guid.str() << std::endl;
      FilterMemberType* member_type = member_types.at(guid.str()).get();
      SerializedPayload payload_copy(payload.length);
      if (!payload_copy.copy(&payload)) {
        std::cerr << "Could not copy serialized payload." << std::endl;
        return false;
      }
      if (!member_type->pubsub_type->deserialize(&payload_copy, member_type->dyn_data)) {
        std::cerr << "Could not deserialize payload to dynamic type" << std::endl;
        return false;
      }

      // For nested structs, this is false if @key is inside the nested one
      // Skip this check, otherwise no filtering is possible for such types
      /*if (!member_type->pubsub_type->m_isGetKeyDefined) {
        std::cerr << "Type has not GetKeyDefined - A nested member?" << std::endl;
        return true; // Keep sample
      } else */
      {
        bool ok_conversion =
          ddsfmu::Converter::fastdds_to_xtypes(member_type->dyn_data, member_type->sample_data);
        if (!ok_conversion) { return false; }
        return member_type->compare_keys(); // Key comparison is done here
      }

    } catch (const std::out_of_range& e) {
      // DataReader in question is not registered and thus irrelevant
    }

    return false; // return false if sample is rejected, true otherwise
  }

}
}
