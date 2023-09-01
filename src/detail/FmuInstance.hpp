#pragma once

#include <filesystem>
#include <functional>
#include <iostream>

#include <cppfmu_cs.hpp>

#include "DataMapper.hpp"
#include "DynamicPubSub.hpp"

namespace ddsfmu {

/**
   @brief Co-simulation slave instance for dds-fmu

   An instance of this class maps directly to C functions defined in the FMI standard.

   It holds members of classes DataMapper and DynamicPubSub, which together ensure mapping
   of signals between FMI and DDS.

*/
class FmuInstance : public cppfmu::SlaveInstance {
public:
  FmuInstance(const std::filesystem::path& resource_path) : m_resource_path(resource_path) {
    FmuInstance::Reset();
  }

  void SetReal(
    const cppfmu::FMIValueReference vr[], std::size_t nvr, const cppfmu::FMIReal value[]) override {
    for (std::size_t i = 0; i < nvr; ++i) { m_mapper.set_double(vr[i], value[i]); }
  }

  inline void GetReal(
    const cppfmu::FMIValueReference vr[], std::size_t nvr, cppfmu::FMIReal value[]) const override {
    for (std::size_t i = 0; i < nvr; ++i) { m_mapper.get_double(vr[i], value[i]); }
  }

  void SetInteger(
    const cppfmu::FMIValueReference vr[], std::size_t nvr,
    const cppfmu::FMIInteger value[]) override {
    for (std::size_t i = 0; i < nvr; ++i) { m_mapper.set_int(vr[i], value[i]); }
  }

  void GetInteger(const cppfmu::FMIValueReference vr[], std::size_t nvr, cppfmu::FMIInteger value[])
    const override {
    for (std::size_t i = 0; i < nvr; ++i) { m_mapper.get_int(vr[i], value[i]); }
  }

  void SetBoolean(
    const cppfmu::FMIValueReference vr[], std::size_t nvr,
    const cppfmu::FMIBoolean value[]) override {
    for (std::size_t i = 0; i < nvr; ++i) { m_mapper.set_bool(vr[i], static_cast<bool>(value[i])); }
  }

  void GetBoolean(const cppfmu::FMIValueReference vr[], std::size_t nvr, cppfmu::FMIBoolean value[])
    const override {
    for (std::size_t i = 0; i < nvr; ++i) {
      bool val;
      m_mapper.get_bool(vr[i], val);
      value[i] = val;
    }
  }

  void SetString(
    const cppfmu::FMIValueReference vr[], std::size_t nvr,
    const cppfmu::FMIString value[]) override {
    for (std::size_t i = 0; i < nvr; ++i) { m_mapper.set_string(vr[i], value[i]); }
  }

  void GetString(const cppfmu::FMIValueReference vr[], std::size_t nvr, cppfmu::FMIString value[])
    const override {
    for (std::size_t i = 0; i < nvr; ++i) {
      std::string val;
      m_mapper.get_string(vr[i], val);
      value[i] = val.c_str();
    }
  }

  ~FmuInstance() = default;

  void SetupExperiment(
    cppfmu::FMIBoolean /*toleranceDefined*/, cppfmu::FMIReal /*tolerance*/, cppfmu::FMIReal tStart,
    cppfmu::FMIBoolean /*stopTimeDefined*/, cppfmu::FMIReal /*tStop*/) override {
    m_time = tStart;
  }

  bool DoStep(
    cppfmu::FMIReal currentCommunicationPoint, cppfmu::FMIReal communicationStepSize,
    cppfmu::FMIBoolean /*newStep*/, cppfmu::FMIReal& /*endOfStep*/) override {
    m_time = currentCommunicationPoint + communicationStepSize;
    m_pubsub.write();
    m_pubsub.take();

    return true;
  }

  void Reset() override {
    m_time = 0.0;
    m_mapper.reset(m_resource_path);
    m_pubsub.reset(m_resource_path, &m_mapper);
  }

private:
  cppfmu::FMIReal m_time;
  std::filesystem::path m_resource_path;
  ddsfmu::DataMapper m_mapper;
  ddsfmu::DynamicPubSub m_pubsub;
};

}
