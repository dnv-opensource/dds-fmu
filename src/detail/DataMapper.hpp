#pragma once

/*
  Copyright 2023, SINTEF Ocean
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <filesystem>
#include <map>
#include <queue>
#include <string>
#include <tuple>

#include <xtypes/DynamicData.hpp>
#include <xtypes/idl/idl.hpp>

#include "visitors.hpp"

namespace ddsfmu {

/**
   @brief Manages mapping between FMU signals and xtypes data storage

   Types defined in IDL is mapped onto four FMU types, namely: Real, Integer, Boolean and
   String. Integer types with more than 32 bits are mapped to Real. The primitive types:
   uint32_t, int64_t, and uint64_t are all mapped to Real. Enumerations are mapped to
   Integer. All data are stored in xtypes::DynamicData. Each data member of DynamicData is
   directly written to or read from using visitor functions, which use references. The
   visitor functions are called from specialized setters and getters:

   set_double(), get_double(), set_int(), get_int(), set_bool(), get_bool(), set_string(), get_string()

*/
class DataMapper {
public:
  /**
     Internal indication whether the mapped signal it is intended for reading (FMU output)
     or writing (FMU input). Parameter is a special case for @key filtering used by readers.
  */
  enum class Direction {
    Read,     ///< Read from DDS, FMU output
    Write,    ///< Write to DDS, FMU input
    Parameter ///< Used for Read content filter of @key
  };

  /**
     Start index offsets for readers/writers functions for a given topic and direction.
     This is useful if you know all the data types for a type and want to access readers/writers directly.
     Order of indexes in tuple: double, integer, bool, string
     See index_offsets().
  */
  typedef std::tuple<std::int32_t, std::int32_t, std::int32_t, std::int32_t> IndexOffsets;

  DataMapper() = default;
  DataMapper(const DataMapper&) = delete;            ///< Copy constructor
  DataMapper& operator=(const DataMapper&) = delete; ///< Copy assignment

  /**
     @brief Clears and repopulates internal data structures

     Clears member variables and containers by first calling clear().
     Loads IDL files into xtypes and uses dds to fmu mapping configuration file to set up mapping between DDS topics to FMU scalar signals.

     @param [in] fmu_resources Path to resources folder of FMU
  */
  void reset(const std::filesystem::path& fmu_resources);

  inline void set_double(const std::int32_t value_ref, const double& value) {
    m_real_writer.at(value_ref)(value);
  }
  inline void get_double(const std::int32_t value_ref, double& value) const {
    m_real_reader.at(value_ref)(value);
  }
  inline void set_int(const std::int32_t value_ref, const std::int32_t& value) {
    m_int_writer.at(value_ref)(value);
  }
  inline void get_int(const std::int32_t value_ref, std::int32_t& value) const {
    m_int_reader.at(value_ref)(value);
  }
  inline void set_bool(const std::int32_t value_ref, const bool& value) {
    m_bool_writer.at(value_ref)(value);
  }
  inline void get_bool(const std::int32_t value_ref, bool& value) const {
    m_bool_reader.at(value_ref)(value);
  }
  inline void set_string(const std::int32_t value_ref, const std::string& value) {
    m_string_writer.at(value_ref)(value);
  }
  inline void get_string(const std::int32_t value_ref, std::string& value) const {
    m_string_reader.at(value_ref)(value);
  }

  inline eprosima::xtypes::DynamicData& data_ref(const std::string& topic, Direction read_write_param) {
    return m_data_store.at(std::make_tuple(topic, read_write_param));
  }
  inline const eprosima::xtypes::DynamicData&
    data_ref(const std::string& topic, Direction read_write_param) const {
    return m_data_store.at(std::make_tuple(topic, read_write_param));
  }

  inline eprosima::xtypes::idl::Context& idl_context() { return m_context; }

  inline IndexOffsets index_offsets(const std::string& topic, Direction read_write_param) const {
    // We have the same number of readers and writers, so the same index applies to both
    return m_offsets.at(std::make_tuple(topic, read_write_param));
  }

  inline void queue_for_key_parameter(const std::string& topic_name, const std::string& topic_type){
    m_potential_keys.push(std::make_pair(topic_name, topic_type));
  }

  void process_key_queue();

private:
  typedef std::tuple<std::string, Direction> StoreKey;
  void add(const std::string& topic_name, const std::string& topic_type, Direction read_write_param);
  void clear(); ///< Clears internal data structures
  std::int32_t m_int_offset, m_real_offset, m_bool_offset, m_string_offset;
  std::map<StoreKey, IndexOffsets> m_offsets;
  std::queue<std::pair<std::string, std::string>> m_potential_keys;
  std::vector<std::function<void(const std::int32_t&)>> m_int_writer;
  std::vector<std::function<void(std::int32_t&)>> m_int_reader;
  std::vector<std::function<void(const double&)>> m_real_writer;
  std::vector<std::function<void(double&)>> m_real_reader;
  std::vector<std::function<void(const bool&)>> m_bool_writer;
  std::vector<std::function<void(bool&)>> m_bool_reader;
  std::vector<std::function<void(const std::string&)>> m_string_writer;
  std::vector<std::function<void(std::string&)>> m_string_reader;
  std::map<StoreKey, eprosima::xtypes::DynamicData> m_data_store;
  eprosima::xtypes::idl::Context m_context;
};

}
