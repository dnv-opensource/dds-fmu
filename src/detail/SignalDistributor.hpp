#pragma once

#include <cstdint>
#include <filesystem>
#include <tuple>
#include <vector>

#include <xtypes/idl/idl.hpp>

#include "auxiliaries.hpp"
#include "model-descriptor.hpp"

typedef std::tuple<std::uint32_t, std::string, std::string, ddsfmu::ScalarVariableType> SignalInfo;

ddsfmu::ScalarVariableType resolve_type(const eprosima::xtypes::DynamicData::ReadableNode& node);

class SignalDistributor {
public:
  SignalDistributor();
  void load_idls(const std::filesystem::path& resource_path);
  bool has_structure(const std::string& topic_type);
  void add(const std::string& topic_name, const std::string& topic_type, bool is_in_not_out);
  inline const std::vector<SignalInfo>& get_mapping() const { return m_signal_mapping; }
  inline std::uint32_t outputs() const { return m_outputs; }
private:
  std::uint32_t  m_real_idx, m_integer_idx, m_boolean_idx, m_string_idx, m_outputs;
  eprosima::xtypes::idl::Context m_context;
  std::vector<SignalInfo> m_signal_mapping;

};
