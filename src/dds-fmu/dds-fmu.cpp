// Copyright 2023-2023, SINTEF Ocean.
// Distributed under the 3-Clause BSD License.
// See accompanying file LICENSE

#include <cmath>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <regex>

#include <cppfmu_cs.hpp>

#include "DdsLoader.hpp"
#include "DynamicPublisher.hpp"
#include "DynamicSubscriber.hpp"
#include "auxiliaries.hpp"

// Temporary
#include <functional>
#include <vector>

enum
  {
    // outputs -> DDS inputs, i.e. subs
    VR_y    = 0,
    VR_OUTPUT_COUNT = 1,

    // inputs and parameters -> DDS outputs, i.e. pubs
    VR_x    = 1,
    VR_COUNT = 2,
  };


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
      // evaluate visitor writer v_{i,R}(vr[i], value[i])
      // auto v_iR = std::function<void(const std::uint32_t, const double&)>
    }

    for (std::size_t i = 0; i < nvr; ++i) {
      if (vr[i] < VR_OUTPUT_COUNT) {
        throw std::out_of_range{"Value reference out of range"};
      }
      m_input[vr[i] - VR_OUTPUT_COUNT] = value[i];
      // Populates data structure to be published on DDS
    }
  }

  void GetReal(
      const cppfmu::FMIValueReference vr[],
      std::size_t nvr,
      cppfmu::FMIReal value[])
    const override
  {

    for (std::size_t i = 0; i < nvr; ++i) {
      // evaluate visitor reader value[i] = v_{o,R}(vr[i])
      // slight rewrite to get more alike function signature
      // auto v_oR = std::function<void(const std::uint32_t, double&)>
    }

    for (std::size_t i = 0; i < nvr; ++i) {
      if (vr[i] == VR_y) {
        value[i] = m_output[VR_y];
        // Use data acquired from DDS

      } else if (vr[i] >= VR_OUTPUT_COUNT) {
        value[i] = m_input[vr[i] - VR_OUTPUT_COUNT];
      } else {
        throw std::out_of_range{"Value reference out of range"};
      }
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
    m_publisher.init(m_loader, m_resource_path);
    m_subscriber.init(m_loader, m_resource_path);
  }

  bool DoStep(
      cppfmu::FMIReal currentCommunicationPoint,
      cppfmu::FMIReal communicationStepSize,
      cppfmu::FMIBoolean /*newStep*/,
      cppfmu::FMIReal& /*endOfStep*/)
    override
  {
    m_time = currentCommunicationPoint + communicationStepSize;

    //Trig trig;
    //trig.sine() = m_input[VR_x];
    //m_trigger_happy.publish(trig);

    // m_input or replacement to be pointed to by publisher
    //
    m_publisher.publish(false);

    //auto& rig = m_trigger_sad.read();
    //m_output[VR_y - VR_INPUT_COUNT] = rig.sine();

    return true;
  }

  void Reset() override
  {
    m_time = 0.0;
    for (auto& v : m_input) v = 0.0;
    for (auto& v : m_output) v = 0.0;
    // Probably define helper functions instead of iterating visitors
    // Reset other types too
  }

  cppfmu::FMIReal m_time;

  cppfmu::FMIReal m_input[VR_COUNT - VR_OUTPUT_COUNT];
  cppfmu::FMIReal m_output[VR_OUTPUT_COUNT];
  DdsLoader m_loader;
  DynamicPublisher m_publisher;   // own visitor writers v_{i,T}, m_publisher.set(const uint32_t, const T&)
  DynamicSubscriber m_subscriber; // own visitor readers v_{o,T}, m_publisher.get(const uint32_t, T&)
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
      std::regex_replace(
          fmuResourceLocation,
          std::regex("file://"), ""));
  auto fmu_base_path = resource_dir.parent_path();
  auto evalGUID = generate_uuid(get_uuid_files(fmu_base_path, false));

  if (evalGUID != std::string(fmuGUID)) {
    throw std::runtime_error(std::string("FMU GUID mismatch: Got from ModelDescription: ")
     + std::string(fmuGUID) + std::string(", but evaluated: ") + evalGUID);
  }

  return cppfmu::AllocateUnique<DdsFmuInstance>(memory, resource_dir);
}
