#pragma once

#include <xtypes/DynamicData.hpp>

template <typename OutType, typename InType>
void reader_visitor(OutType& out, const eprosima::xtypes::ReadableDynamicDataRef& cref){
  out = static_cast<OutType>(cref.value<InType>()); // should use numeric_cast
}

template <typename InType, typename OutType>
void writer_visitor(const InType& in, eprosima::xtypes::WritableDynamicDataRef& ref){
  ref.value(static_cast<OutType>(in)); // should use numeric_cast
}
