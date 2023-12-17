#pragma once

#include <fastdds/dds/topic/IContentFilter.hpp>
#include <fastdds/dds/topic/IContentFilterFactory.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>

#include "Converter.hpp"
#include "CustomKeyFilter.hpp"

namespace ddsfmu {
namespace detail {

class CustomKeyFilterFactory : public eprosima::fastdds::dds::IContentFilterFactory {
public:
  /**
    @brief Create a ContentFilteredTopic using this factory.

    Updating the filter will not delete the old one. Once a reader is added, it cannot be
    removed.

    @param filter_class_name Custom filter name
    @param type_name Data type name
    @param filter_parameters Parameters required by the filter
    @param filter_instance Instance of the filter to be evaluated

    @return eprosima::fastrtps::types::ReturnCode_t::RETCODE_BAD_PARAMETER if the requirements for creating the
            ContentFilteredTopic using this factory are not met
            eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK if the ContentFilteredTopic is correctly created
   */
  eprosima::fastrtps::types::ReturnCode_t create_content_filter(
    const char* filter_class_name, // Custom filter class name is 'CUSTOM_KEY_FILTER'.
    const char* type_name,         // Type name of dynamic type
    const eprosima::fastdds::dds::TopicDataType* data_type, // This is a DynamicPubSubType*
    const char* /*filter_expression*/, // This Custom Filter doesn't implement a filter expression.
    const ParameterSeq& filter_parameters, // The GUID and key parameters
    eprosima::fastdds::dds::IContentFilter*& filter_instance) override {
    // Check the ContentFilteredTopic should be created by this factory.
    if (std::string(filter_class_name) != "CUSTOM_KEY_FILTER") {
      return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }

    if (filter_parameters.length() < 1) { return ReturnCode_t::RETCODE_BAD_PARAMETER; }

    if (filter_instance == nullptr) {
      try {
        filter_instance = new CustomKeyFilter(data_type, type_name, filter_parameters);
      } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
      }
    }

    if (std::string(filter_parameters[0]) != "|GUID UNKNOWN|") {
      try {
        CustomKeyFilter* instance = dynamic_cast<CustomKeyFilter*>(filter_instance);

        // Only adds a reader once. Once added, changing filter_parameters has not effect
        if (!instance->has_reader_GUID(filter_parameters[0])) {
          instance->add_type(data_type, type_name, filter_parameters);
        }
      } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
      }
    }

    //if (filter_instance != nullptr) { delete (dynamic_cast<CustomKeyFilter*>(filter_instance)); }
    return ReturnCode_t::RETCODE_OK;
  }

  /**
   * @brief Delete a ContentFilteredTopic created by this factory
   *
   * @param filter_class_name Custom filter name
   * @param filter_instance Instance of the filter to be deleted.
   *                        After returning, the passed pointer becomes invalid.
   * @return eprosima::fastrtps::types::ReturnCode_t::RETCODE_BAD_PARAMETER if the instance was created with another
   *         factory
   *         eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK if correctly deleted
   */
  eprosima::fastrtps::types::ReturnCode_t delete_content_filter(
    const char* filter_class_name,
    eprosima::fastdds::dds::IContentFilter* filter_instance) override {
    if (std::string(filter_class_name) != "CUSTOM_KEY_FILTER" || !filter_instance) {
      return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }

    delete (dynamic_cast<CustomKeyFilter*>(filter_instance));
    return ReturnCode_t::RETCODE_OK;
  }
};

}
}
