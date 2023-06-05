// Copyright 2023-2023, SINTEF Ocean.
// Distributed under the 3-Clause BSD License.
// See accompanying file LICENSE

#include <cmath>
#include <cstring>
#include <iostream>
#include <filesystem>
#include <regex>

#include <cppfmu_cs.hpp>

#include "trigPubSubTypes.h"
#include "TrigPublisher.hpp"
#include "TrigSubscriber.hpp"

#include "auxiliaries.hpp"

enum
{
    // inputs and parameters -> DDS outputs, i.e. pubs
    VR_x    = 0,
    VR_INPUT_COUNT = 1,

    // outputs -> DDS inputs, i.e. subs
    VR_y    = 1,

    VR_COUNT = 2,
};


class DdsFmuInstance : public cppfmu::SlaveInstance
{
public:
    DdsFmuInstance()
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
            if (vr[i] >= VR_INPUT_COUNT) {
                throw std::out_of_range{"Value reference out of range"};
            }
            m_input[vr[i]] = value[i];
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
            if (vr[i] == VR_y) {
                value[i] = m_output[VR_y - VR_INPUT_COUNT];
                // Use data acquired from DDS

            } else if (vr[i] < VR_INPUT_COUNT) {
                value[i] = m_input[vr[i]];
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
        m_trigger_happy.init(false);
        m_trigger_sad.init(false);
    }

    bool DoStep(
        cppfmu::FMIReal currentCommunicationPoint,
        cppfmu::FMIReal communicationStepSize,
        cppfmu::FMIBoolean /*newStep*/,
        cppfmu::FMIReal& /*endOfStep*/)
        override
    {
        m_time = currentCommunicationPoint + communicationStepSize;
        Trig trig;
        trig.sine() = m_input[VR_x];
        m_trigger_happy.publish(trig);

        auto& rig = m_trigger_sad.read();
        m_output[VR_y - VR_INPUT_COUNT] = rig.sine();

        return true;
    }

    void Reset() override
    {
        m_time = 0.0;
        for (auto& v : m_input) v = 0.0;
        for (auto& v : m_output) v = 0.0;
    }

    // cppfmu::FMIReal Calculate() const CPPFMU_NOEXCEPT
    // {
    //     return m_input[VR_w] * m_input[VR_x];
    // }

    cppfmu::FMIReal m_time;
    cppfmu::FMIReal m_input[VR_INPUT_COUNT];
    cppfmu::FMIReal m_output[VR_COUNT - VR_INPUT_COUNT];
    // Add other types as well
    TrigPublisher m_trigger_happy;
    TrigSubscriber m_trigger_sad;

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

    namespace fs = std::filesystem;
    auto resource_dir = fs::path(std::regex_replace(fmuResourceLocation, std::regex("file://"), ""));
    auto fmu_base_path = resource_dir.parent_path();

    auto evalGUID = generate_uuid(get_uuid_files(fmu_base_path));

    if (evalGUID != std::string(fmuGUID)) {
      throw std::runtime_error("FMU GUID mismatch");
    }

    return cppfmu::AllocateUnique<DdsFmuInstance>(memory);
}
