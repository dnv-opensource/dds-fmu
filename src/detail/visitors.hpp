#pragma once


namespace ddsfmu {

/**
   @namespace ddsfmu::detail Helper functions for loading and generating configuration files
*/
namespace detail {


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

}
}
