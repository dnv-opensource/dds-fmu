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
template<typename OutType, typename InType>
void reader_visitor(OutType& out, const eprosima::xtypes::ReadableDynamicDataRef& cref) {
  out = static_cast<OutType>(cref.value<InType>()); // should use numeric_cast
}

/**
   @brief A writer visitor sends a value to an xtypes data reference
*/
template<typename InType, typename OutType>
void writer_visitor(const InType& in, eprosima::xtypes::WritableDynamicDataRef& ref) {
  ref.value(static_cast<OutType>(in)); // should use numeric_cast
}

template<>
inline void reader_visitor<std::string, char>(
  std::string& out, const eprosima::xtypes::ReadableDynamicDataRef& cref) {
  out = std::string(1, cref.value<char>());
}

template<>
inline void writer_visitor<std::string, char>(
  const std::string& in, eprosima::xtypes::WritableDynamicDataRef& ref) {
  ref.value(in[0]);
}

/**
   @brief Manages mapping between FMU signals and xtypes data storage

   Types defined in IDL is mapped onto four FMU types, namely (Real, Integer, Boolean and String).
   Integer types with >32 bit is mapped to Real. uint32_t, int64_t, uint64_t are mapped to Real. Enumeration is mapped to Integer.
   All data are stored in xtypes::DynamicData. Each data member of DynamicData is directly written to or read from using visitor functions.
   The visitor functions are called from {set,get}_{double,int,bool,string}(value_ref, value).

*/
class DataMapper {
public:
  enum Direction {
    Read,
    Write
  }; ///< Internal indication whether it is Read (FMU output) or Write (FMU input)

  DataMapper() = default;
  DataMapper(const DataMapper&) = delete;            ///< Copy constructor
  DataMapper& operator=(const DataMapper&) = delete; ///< Copy assignment

  /**
     @brief Clears and repopulates internal data structures

     Clears member variables and containers.
     Loads IDL files into xtypes and uses dds to fmu mapping configuration file to set up mapping between DDS topics to FMU scalar signals.

     @param [in] fmu_resources Path to resources folder of FMU
  */
  void reset(const std::filesystem::path& fmu_resources);

  // visitor functions
  inline void set_double(const std::int32_t value_ref, const double& value) {
    m_real_writer.at(value_ref - m_real_offset)(value);
  }
  void get_double(const std::int32_t value_ref, double& value) const {
    m_real_reader.at(value_ref)(value);
  }
  void set_int(const std::int32_t value_ref, const std::int32_t& value) {
    m_int_writer.at(value_ref - m_int_offset)(value);
  }
  void get_int(const std::int32_t value_ref, std::int32_t& value) const {
    m_int_reader.at(value_ref)(value);
  }
  void set_bool(const std::int32_t value_ref, const bool& value) {
    m_bool_writer.at(value_ref - m_bool_offset)(value);
  }
  void get_bool(const std::int32_t value_ref, bool& value) const {
    m_bool_reader.at(value_ref)(value);
  }
  void set_string(const std::int32_t value_ref, const std::string& value) {
    m_string_writer.at(value_ref - m_string_offset)(value);
  }
  void get_string(const std::int32_t value_ref, std::string& value) const {
    m_string_reader.at(value_ref)(value);
  }

  inline eprosima::xtypes::DynamicData& data_ref(const std::string& topic, Direction read_write) {
    return m_data_store.at(std::make_tuple(topic, read_write));
  }
  inline const eprosima::xtypes::DynamicData&
    data_ref(const std::string& topic, Direction read_write) const {
    return m_data_store.at(std::make_tuple(topic, read_write));
  }

  inline eprosima::xtypes::idl::Context& idl_context() { return m_context; }

  inline std::int32_t int_offset() { return m_int_offset; }
  inline std::int32_t real_offset() { return m_real_offset; }
  inline std::int32_t bool_offset() { return m_bool_offset; }
  inline std::int32_t string_offset() { return m_string_offset; }

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

  std::int32_t m_int_offset, m_real_offset, m_bool_offset, m_string_offset;

  std::map<StoreKey, eprosima::xtypes::DynamicData>
    m_data_store; // TODO: key needs extending to support key
  eprosima::xtypes::idl::Context m_context;
};
