#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <tuple>

#include <xtypes/DynamicData.hpp>
#include <xtypes/idl/idl.hpp>


/**
   @brief A reader visitor retrieves a value from an xtypes data reference
*/
template <typename OutType, typename InType>
void reader_visitor(OutType& out, const eprosima::xtypes::ReadableDynamicDataRef& cref){
  out = static_cast<OutType>(cref.value<InType>()); // should use numeric_cast
}

/**
   @brief A writer visitor sends a value to an xtypes data reference
*/
template <typename InType, typename OutType>
void writer_visitor(const InType& in, eprosima::xtypes::WritableDynamicDataRef& ref){
  ref.value(static_cast<OutType>(in)); // should use numeric_cast
}

template <>
inline void reader_visitor<std::string, char>(std::string& out, const eprosima::xtypes::ReadableDynamicDataRef& cref){
  out = std::string(1, cref.value<char>());
}

template <>
inline void writer_visitor<std::string, char>(const std::string& in, eprosima::xtypes::WritableDynamicDataRef& ref){
  ref.value(in[0]);
}

/**
   @brief Manages mapping between FMU signals and xtypes data storage


*/
class DataMapper {
public:
  enum Direction { Read, Write };

  DataMapper() = default;
  DataMapper(const DataMapper&) = delete;
  DataMapper& operator=(const DataMapper&) = delete;

  void reset(const std::filesystem::path& fmu_root);

  // visitor functions
  void set_double(const std::int32_t value_ref, const double& value);
  void get_double(const std::int32_t value_ref, double& value) const;
  void set_int(const std::int32_t value_ref, const std::int32_t& value);
  void get_int(const std::int32_t value_ref, std::int32_t& value) const;
  void set_bool(const std::int32_t value_ref, const bool& value);
  void get_bool(const std::int32_t value_ref, bool& value) const;
  void set_string(const std::int32_t value_ref, const std::string& value);
  void get_string(const std::int32_t value_ref, std::string& value) const;

  inline eprosima::xtypes::DynamicData& data_ref(const std::string& topic, Direction read_write) {
    return m_data_store.at(std::make_tuple(topic, read_write));
  }
  inline const eprosima::xtypes::DynamicData& data_ref(const std::string& topic, Direction read_write) const {
    return m_data_store.at(std::make_tuple(topic, read_write)); }

private:
  typedef std::tuple<std::string, Direction> StoreKey;
  void clear();
  void add(const std::string& topic_name, const std::string& topic_type, Direction read_write);

  std::vector<std::function<void(const std::int32_t&)>> m_int_writer;
  std::vector<std::function<void(std::int32_t&)>> m_int_reader;
  std::vector<std::function<void(const double&)>> m_real_writer;
  std::vector<std::function<void(double&)>> m_real_reader;
  std::vector<std::function<void(const bool&)>> m_bool_writer;
  std::vector<std::function<void(bool&)>> m_bool_reader;
  std::vector<std::function<void(const std::string&)>> m_string_writer;
  std::vector<std::function<void(std::string&)>> m_string_reader;

  std::map<StoreKey, eprosima::xtypes::DynamicData> m_data_store; // TODO: key needs extending to support key
  eprosima::xtypes::idl::Context m_context;

};
