#pragma once

/*
 * Copyright 2019 - present Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * This implementation is based on https://github.com/eProsima/FastDDS-SH/blob/main/src/Conversion.hpp
 * Licensed under the Apache License, Version 2.0 (the "License");
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include <map>
#include <vector>

#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/DynamicPubSubType.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>
#include <xtypes/idl/idl.hpp>
#include <xtypes/xtypes.hpp>

namespace ddsfmu {

/**
   @brief Class with functions to convert between xtypes::DynamicData and fast-dds DynamicData
*/
struct Converter {
  /**
       @brief Converts from xtypes to fastdds DynamicData

       This function takes an xtypes DynamicData reference and converts it into a fast-dds
       DynamicData pointer. If the conversion is successful, the function returns true.

       @param [in] input xtypes DynamicData reference
       @param [out] output fastdds DynamicData pointer
       @return Boolean on result of operation
    */
  static bool xtypes_to_fastdds(
    const eprosima::xtypes::DynamicData& input, eprosima::fastrtps::types::DynamicData* output);

  /**
       @brief Converts from fastdds to xtypes DynamicData

       This function takes an fast-dds DynamicData pointer and converts it into an xtypes
       DynamicData reference. If the conversion is successful, the function returns true.

       @param [in] input fastdds DynamicData pointer
       @param [out] output xtypes DynamicData reference
       @return Boolean on result of operation
    */
  static bool fastdds_to_xtypes(
    const eprosima::fastrtps::types::DynamicData* input, eprosima::xtypes::DynamicData& output);

  /**
       @brief  Retrieve a dynamic data instance given type name

       Get dynamic data given type name. This function looks up the dynamic typein a map of registered eprosima::xtypes::DynamicType::Ptr.

       @param [in] type_name Name of type to retrieve
       @return Instance of DynamicData

    */
  static eprosima::xtypes::DynamicData dynamic_data(const std::string& type_name);

  /**
       @brief Registers a fastdds pub sub type with provided type name

       Registers DynamicPubSubType in a map, associating it with the given type name.

       @param [in] type_name Name of type to register
       @param [in] type Pointer to dynamic type to be registered

    */
  static void register_type(
    const std::string& type_name, eprosima::fastrtps::types::DynamicPubSubType* type) {
    m_registered_types.emplace(type_name, type);
  }

  /**
       @brief Return fastdds DynamicTypeBuilder given xtypes DynamicType

       Retrieves a DynamicTypeBuilder and internally creates it if not already created by the DynamicTypeBuilderFactory. The xtypes dynamic type is converted into fast-dds dynamic type. This function makes necessary API calls to ensure that many xtypes type kinds are mapped out as analogous fast-dds dynamic types, including arrays, structured types, and union types.

       @param [in] type xtypes DynamicType for which to create a DynamicTypeBuilder
       @return DynamicTypeBuilder pointer

    */
  static eprosima::fastrtps::types::DynamicTypeBuilder*
    create_builder(const eprosima::xtypes::DynamicType& type);

  /**
       @brief Patches type names with '/' in their type name

       Replaces '/' with "__" and returns the patched name.

       @param [in] message_type Type name
       @return Patched type name

    */
  static std::string convert_type_name(const std::string& message_type);

  /**
       @brief Retrieve member type of aggregated type

       Iteratively resolves member types in discriminator string until deepest member is
       found. The members are separated by '.'. Returns the DynamicType of this member.

       @param [in] service_type Type with member type
       @param [in] discriminator Nested name of member

       @return Resolved DynamicType
    */
  static const eprosima::xtypes::DynamicType& resolve_discriminator_type(
    const eprosima::xtypes::DynamicType& service_type, const std::string& discriminator);

  /**
       @brief Retrieve a writable dynamic data reference member

       Recursively navigate dynamic data ref until leaf member is found.
       Returns the corresponding writable dynamic data reference.

       @param [in] membered_data Instance of data reference to search in
       @param [in] path Path to member with '.' to separate members.

    */
  static eprosima::xtypes::WritableDynamicDataRef access_member_data(
    eprosima::xtypes::WritableDynamicDataRef membered_data, const std::string& path);

  /**
     @brief Clear converter data structures

     Clear internal std::maps. It is useful to call this before deleting DDS participants
     to avoid invalid reads of deleted log resources.

  */
  static void clear_data_structures() {
    m_types.clear();
    m_registered_types.clear();
    m_builders.clear();
  }

private:
  ~Converter() = default;
  static std::map<std::string, eprosima::xtypes::DynamicType::Ptr> m_types;
  static std::map<std::string, eprosima::fastrtps::types::DynamicPubSubType*> m_registered_types;
  static std::map<std::string, eprosima::fastrtps::types::DynamicTypeBuilder_ptr> m_builders;

  static const eprosima::xtypes::DynamicType&
    resolve_type(const eprosima::xtypes::DynamicType& type);
  static eprosima::fastrtps::types::TypeKind
    resolve_type(const eprosima::fastrtps::types::DynamicType_ptr type);
  static eprosima::fastrtps::types::DynamicTypeBuilder_ptr
    get_builder(const eprosima::xtypes::DynamicType& type);

  static void get_array_specs(
    const eprosima::xtypes::ArrayType& array,
    std::pair<std::vector<uint32_t>, eprosima::fastrtps::types::DynamicTypeBuilder_ptr>& result);

  static eprosima::xtypes::WritableDynamicDataRef access_member_data(
    eprosima::xtypes::WritableDynamicDataRef membered_data, const std::vector<std::string>& tokens,
    size_t index);

  // xtypes Dynamic Data -> FastDDS Dynamic Data
  static void set_primitive_data(
    eprosima::xtypes::ReadableDynamicDataRef from, eprosima::fastrtps::types::DynamicData* to,
    eprosima::fastrtps::types::MemberId id);
  static void set_sequence_data(
    eprosima::xtypes::ReadableDynamicDataRef from, eprosima::fastrtps::types::DynamicData* to);
  static void set_map_data(
    eprosima::xtypes::ReadableDynamicDataRef from, eprosima::fastrtps::types::DynamicData* to);
  static void set_array_data(
    eprosima::xtypes::ReadableDynamicDataRef from, eprosima::fastrtps::types::DynamicData* to,
    const std::vector<uint32_t>& indexes);
  static bool set_struct_data(
    eprosima::xtypes::ReadableDynamicDataRef input, eprosima::fastrtps::types::DynamicData* output);
  static bool set_union_data(
    eprosima::xtypes::ReadableDynamicDataRef input, eprosima::fastrtps::types::DynamicData* output);


  // FastDDS Dynamic Data -> xtypes Dynamic Data
  static void set_sequence_data(
    const eprosima::fastrtps::types::DynamicData* from,
    eprosima::xtypes::WritableDynamicDataRef to);
  static void set_map_data(
    const eprosima::fastrtps::types::DynamicData* from,
    eprosima::xtypes::WritableDynamicDataRef to);
  static void set_array_data(
    const eprosima::fastrtps::types::DynamicData* from, eprosima::xtypes::WritableDynamicDataRef to,
    const std::vector<uint32_t>& indexes);
  static bool set_struct_data(
    const eprosima::fastrtps::types::DynamicData* input,
    eprosima::xtypes::WritableDynamicDataRef output);
  static bool set_union_data(
    const eprosima::fastrtps::types::DynamicData* input,
    eprosima::xtypes::WritableDynamicDataRef output);
};

}
