// Copyright 2023-2023, SINTEF Ocean.
// Distributed under the 3-Clause BSD License.
// See accompanying file LICENSE

#include <cmath>
#include <cstring>
#include <filesystem>
#include <functional>
#include <iostream>
#include <regex>

#include <cppfmu_cs.hpp>

#include "auxiliaries.hpp"
#include "DataMapper.hpp"
#include "DynamicPubSub.hpp"

class DdsFmuInstance : public cppfmu::SlaveInstance
{
public:
  DdsFmuInstance(const std::filesystem::path& resource_path) :
    m_resource_path(resource_path)
  {
    DdsFmuInstance::Reset();
  }

  void SetReal(
      const cppfmu::FMIValueReference vr[],
      std::size_t nvr,
      const cppfmu::FMIReal value[])
    override
  {
    for (std::size_t i = 0; i < nvr; ++i) {
      m_mapper.set_double(vr[i], value[i]);
    }
  }

  void GetReal(
      const cppfmu::FMIValueReference vr[],
      std::size_t nvr,
      cppfmu::FMIReal value[])
    const override
  {

    for (std::size_t i = 0; i < nvr; ++i) {
      m_mapper.get_double(vr[i], value[i]);
    }
  }

  void SetInteger(
      const cppfmu::FMIValueReference vr[],
      std::size_t nvr,
      const cppfmu::FMIInteger value[])
    override
  {
    for (std::size_t i = 0; i < nvr; ++i) {
      m_mapper.set_int(vr[i], value[i]);
    }
  }

  void GetInteger(
      const cppfmu::FMIValueReference vr[],
      std::size_t nvr,
      cppfmu::FMIInteger value[])
    const override
  {
    for (std::size_t i = 0; i < nvr; ++i) {
      m_mapper.get_int(vr[i], value[i]);
    }
  }

  void SetBoolean(
      const cppfmu::FMIValueReference vr[],
      std::size_t nvr,
      const cppfmu::FMIBoolean value[])
    override
  {
    for (std::size_t i = 0; i < nvr; ++i) {
      m_mapper.set_bool(vr[i], static_cast<bool>(value[i]));
    }
  }

  void GetBoolean(
      const cppfmu::FMIValueReference vr[],
      std::size_t nvr,
      cppfmu::FMIBoolean value[])
    const override
  {
    for (std::size_t i = 0; i < nvr; ++i) {
      bool val;
      m_mapper.get_bool(vr[i], val);
      value[i] = val;
    }
  }

  void SetString(
      const cppfmu::FMIValueReference vr[],
      std::size_t nvr,
      const cppfmu::FMIString value[])
    override
  {
    for (std::size_t i = 0; i < nvr; ++i) {
      m_mapper.set_string(vr[i], value[i]);
    }
  }

  void GetString(
      const cppfmu::FMIValueReference vr[],
      std::size_t nvr,
      cppfmu::FMIString value[])
    const override
  {
    for (std::size_t i = 0; i < nvr; ++i) {
      std::string val;
      m_mapper.get_string(vr[i], val);
      value[i] = val.c_str();
    }
  }

private:
  void SetupExperiment(
      cppfmu::FMIBoolean /*toleranceDefined*/,
      cppfmu::FMIReal /*tolerance*/,
      cppfmu::FMIReal tStart,
      cppfmu::FMIBoolean /*stopTimeDefined*/,
      cppfmu::FMIReal /*tStop*/)
    override
  {
    m_time = tStart;
  }

  bool DoStep(
      cppfmu::FMIReal currentCommunicationPoint,
      cppfmu::FMIReal communicationStepSize,
      cppfmu::FMIBoolean /*newStep*/,
      cppfmu::FMIReal& /*endOfStep*/)
    override
  {
    m_time = currentCommunicationPoint + communicationStepSize;
    m_pubsub.write();
    m_pubsub.take();

    return true;
  }

  void Reset() override
  {
    m_time = 0.0;
    m_mapper.reset(m_resource_path);
    m_pubsub.reset(m_resource_path, &m_mapper);
  }

  cppfmu::FMIReal m_time;
  DataMapper m_mapper; // has reader/writer visitor functions into DynamicData
  DynamicPubSub m_pubsub;
  std::filesystem::path m_resource_path;

};


cppfmu::UniquePtr<cppfmu::SlaveInstance> CppfmuInstantiateSlave(
    cppfmu::FMIString  /*instanceName*/,
    cppfmu::FMIString  fmuGUID,
    cppfmu::FMIString  fmuResourceLocation,
    cppfmu::FMIString  /*mimeType*/,
    cppfmu::FMIReal    /*timeout*/,
    cppfmu::FMIBoolean /*visible*/,
    cppfmu::FMIBoolean /*interactive*/,
    cppfmu::Memory memory,
    cppfmu::Logger /*logger*/)
{
  auto resource_dir = std::filesystem::path(
      std::regex_replace(fmuResourceLocation, std::regex("file://"), ""));
  auto fmu_base_path = resource_dir.parent_path();
  auto evalGUID = generate_uuid(get_uuid_files(fmu_base_path, false));

  if (evalGUID != std::string(fmuGUID)) {
    throw std::runtime_error(std::string("FMU GUID mismatch: Got from ModelDescription: ")
     + std::string(fmuGUID) + std::string(", but evaluated: ") + evalGUID);
  }

  return cppfmu::AllocateUnique<DdsFmuInstance>(memory, resource_dir);
}
