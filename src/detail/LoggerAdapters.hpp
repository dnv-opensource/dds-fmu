#pragma once

/*
  Copyright 2023, SINTEF Ocean
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <iostream>
#include <sstream>
#include <string>
#include <utility>

#include <cppfmu_common.hpp>
#include <fastdds/dds/log/OStreamConsumer.hpp>

namespace ddsfmu {

class FmiLogger : public eprosima::fastdds::dds::OStreamConsumer {
public:
  FmiLogger() = delete;
  FmiLogger(cppfmu::Logger& logger, const std::string& name)
      : m_logger(logger), m_str(std::ostringstream::ate), m_name(name) {}
  virtual ~FmiLogger() = default;

  void Consume(const eprosima::fastdds::dds::Log::Entry& entry) override {
    eprosima::fastdds::dds::OStreamConsumer::Consume(entry);

    cppfmu::FMIStatus status;

    switch (entry.kind) {
    case eprosima::fastdds::dds::Log::Kind::Info: status = cppfmu::FMIOK;
    case eprosima::fastdds::dds::Log::Kind::Warning:
    case eprosima::fastdds::dds::Log::Kind::Error: status = cppfmu::FMIWarning;
    }

    m_logger.Log(status, "", m_str.str().c_str()); // TODO: should category be set differently?
    m_str.str("");
    m_str.clear();
  }

protected:
  std::ostream& get_stream(const eprosima::fastdds::dds::Log::Entry& entry) override {
    return m_str;
  }

private:
  std::ostringstream m_str;
  std::string m_name;
  cppfmu::Logger& m_logger;
};

}
