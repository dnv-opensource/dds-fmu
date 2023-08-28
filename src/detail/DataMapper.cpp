
#include <functional>
#include <vector>

#include <rapidxml/rapidxml.hpp>

#include "model-descriptor.hpp"

#include "DataMapper.hpp"
#include "SignalDistributor.hpp" // resolve_type

void DataMapper::clear(){
  m_int_writer.clear();
  m_int_reader.clear();
  m_real_writer.clear();
  m_real_reader.clear();
  m_bool_writer.clear();
  m_bool_reader.clear();
  m_string_writer.clear();
  m_string_reader.clear();
  m_data_store.clear();
  m_int_offset = 0;
  m_real_offset = 0;
  m_bool_offset = 0;
  m_string_offset = 0;
}

void DataMapper::reset(const std::filesystem::path& fmu_resources){

  clear();

  m_context = load_fmu_idls(fmu_resources);

  auto ddsfmu_mapping = fmu_resources / "config" / "dds" / "ddsfmu_mapping.xml";
  std::vector<char> buffer;
  rapidxml::xml_document<> signal_mapping;
  load_ddsfmu_mapping(signal_mapping, ddsfmu_mapping, buffer);

  auto mapper_ddsfmu = signal_mapping.first_node("ddsfmu");

  if(mapper_ddsfmu == nullptr){
    throw std::runtime_error("<ddsfmu> not found in ddsfmu_mapping.xml");
  }

  auto mapper_iterator = [&](bool is_in_not_out) {
    std::string node_name;
    Direction direction;
    if (is_in_not_out) {
      // fmu input aka writer aka Setter aka DDS Publisher
      node_name = "fmu_in";
      direction = Direction::Write;
    } else {
      // fmu output aka reader aka Getter aka DDS Subscriber
      node_name = "fmu_out";
      direction = Direction::Read;
    }

    for (rapidxml::xml_node<>* fmu_node = mapper_ddsfmu->first_node(node_name.c_str());
         fmu_node;
         fmu_node = fmu_node->next_sibling(node_name.c_str())) {

      auto topic = fmu_node->first_attribute("topic");
      auto type = fmu_node->first_attribute("type");
      if(!topic || !type){
        std::cerr << "<ddsfmu><" << node_name
                  << "> must specify attributes 'topic' and 'type'. Got: 'topic': "
                  << std::boolalpha << (topic != nullptr) << " and 'type': "
                  << std::boolalpha << (type != nullptr) << std::endl;
        throw std::runtime_error("Incomplete user data");
      }
      std::string topic_name(topic->value());
      std::string topic_type(type->value());

      if(!m_context.module().has_structure(topic_type)){
        std::cerr << "Got non-existing 'type': " << topic_type << std::endl;
        throw std::runtime_error("Unknown idl type");
      }

      add(topic_name, topic_type, direction);
    }
  };

  mapper_iterator(false);
  // These are used for offsetting value_ref of writers
  m_int_offset = m_int_reader.size();
  m_real_offset = m_real_reader.size();
  m_bool_offset = m_bool_reader.size();
  m_string_offset = m_string_reader.size();
  mapper_iterator(true);

}

void DataMapper::add(const std::string& topic_name, const std::string& topic_type, Direction read_write){

  const eprosima::xtypes::DynamicType& message_type(m_context.module().structure(topic_type));
  DataMapper::StoreKey key = std::make_tuple(topic_name, read_write);

  auto item = m_data_store.emplace(key, eprosima::xtypes::DynamicData(message_type));

  if (!item.second){
    std::string dir;
    if (read_write == DataMapper::Direction::Write){ dir = "input"; }
    else { dir = "output"; }

    throw std::runtime_error(
        std::string("Tried to create existing topic: ") + topic_name + std::string(" for FMU ") + dir);
  }

  auto& dyn_data = (*item.first).second;

  // switch on type kind must be identical to the one in SignalDistributor

  // FMU output
  // for each output: register dynamicdata store for requested type in a map (key: topic ) TODO: support @key
  //   for each type (primitive/string): register reader_visitor (read from dds into fmu, i.e. Get{Real,Integer..})
  //     valueRef is index of registered visitor
  //     Note: we also register reader_visitor for FMU inputs.

  // FMU input
  // for each input: register dynamicdata store for requrest typ in a map (key: topic) TODO: support @key
  //   for each type (primitive/string): register writer_visitor (write from fmu into dds, i.e. Set{Real,Integer..})
  //     valueRef is index of registered visitor (kind of)

  dyn_data.for_each([&](eprosima::xtypes::DynamicData::WritableNode& node){
    bool is_leaf = (node.type().is_primitive_type() || node.type().is_enumerated_type());
    bool is_string = node.type().kind() == eprosima::xtypes::TypeKind::STRING_TYPE;

    if(is_leaf || is_string){

      auto fmi_type = resolve_type(node);

      switch (fmi_type) {
      case ddsfmu::Real:
        switch (node.type().kind())
        {
        case eprosima::xtypes::TypeKind::FLOAT_32_TYPE:
          if(read_write == Direction::Write){
            m_real_writer.emplace_back(
                std::bind(writer_visitor<double, float>, std::placeholders::_1, node.data()));
          }
          m_real_reader.emplace_back(
              std::bind(reader_visitor<double, float>, std::placeholders::_1, node.data()));
          break;
        case eprosima::xtypes::TypeKind::FLOAT_64_TYPE:
          if(read_write == Direction::Write){
            m_real_writer.emplace_back(
                std::bind(writer_visitor<double, double>, std::placeholders::_1, node.data()));
          }
          m_real_reader.emplace_back(
              std::bind(reader_visitor<double, double>, std::placeholders::_1, node.data()));
          break;
        case eprosima::xtypes::TypeKind::INT_64_TYPE:
          if(read_write == Direction::Write){
            m_real_writer.emplace_back(
                std::bind(writer_visitor<double, std::int64_t>, std::placeholders::_1, node.data()));
          }
          m_real_reader.emplace_back(
              std::bind(reader_visitor<double, std::int64_t>, std::placeholders::_1, node.data()));
          break;
        case eprosima::xtypes::TypeKind::UINT_64_TYPE:
          if(read_write == Direction::Write){
            m_real_writer.emplace_back(
                std::bind(writer_visitor<double, std::uint64_t>, std::placeholders::_1, node.data()));
          }
          m_real_reader.emplace_back(
              std::bind(reader_visitor<double, std::uint64_t>, std::placeholders::_1, node.data()));
          break;
        default:
          break;
        }
        break;
      case ddsfmu::Integer:
        switch (node.type().kind())
        {
        case eprosima::xtypes::TypeKind::INT_8_TYPE:
          if(read_write == Direction::Write){
            m_int_writer.emplace_back(
                std::bind(writer_visitor<std::int32_t, std::int8_t>, std::placeholders::_1, node.data()));
          }
          m_int_reader.emplace_back(
              std::bind(reader_visitor<std::int32_t, std::int8_t>, std::placeholders::_1, node.data()));
          break;
        case eprosima::xtypes::TypeKind::UINT_8_TYPE:
          if(read_write == Direction::Write){
            m_int_writer.emplace_back(
                std::bind(writer_visitor<std::int32_t, std::uint8_t>, std::placeholders::_1, node.data()));
          }
          m_int_reader.emplace_back(
              std::bind(reader_visitor<std::int32_t, std::uint8_t>, std::placeholders::_1, node.data()));
          break;
        case eprosima::xtypes::TypeKind::INT_16_TYPE:
          if(read_write == Direction::Write){
            m_int_writer.emplace_back(
                std::bind(writer_visitor<std::int32_t, std::int16_t>, std::placeholders::_1, node.data()));
          }
          m_int_reader.emplace_back(
              std::bind(reader_visitor<std::int32_t, std::int16_t>, std::placeholders::_1, node.data()));
          break;
        case eprosima::xtypes::TypeKind::UINT_16_TYPE:
          if(read_write == Direction::Write){
            m_int_writer.emplace_back(
                std::bind(writer_visitor<std::int32_t, std::uint16_t>, std::placeholders::_1, node.data()));
          }
          m_int_reader.emplace_back(
              std::bind(reader_visitor<std::int32_t, std::uint16_t>, std::placeholders::_1, node.data()));
          break;
        case eprosima::xtypes::TypeKind::INT_32_TYPE:
          if(read_write == Direction::Write){
            m_int_writer.emplace_back(
                std::bind(writer_visitor<std::int32_t, std::int32_t>, std::placeholders::_1, node.data()));
          }
          m_int_reader.emplace_back(
              std::bind(reader_visitor<std::int32_t, std::int32_t>, std::placeholders::_1, node.data()));
          break;
        case eprosima::xtypes::TypeKind::UINT_32_TYPE:
          if(read_write == Direction::Write){
            m_int_writer.emplace_back(
                std::bind(writer_visitor<std::int32_t, std::uint32_t>, std::placeholders::_1, node.data()));
          }
          m_int_reader.emplace_back(
              std::bind(reader_visitor<std::int32_t, std::uint32_t>, std::placeholders::_1, node.data()));
          break;
        case eprosima::xtypes::TypeKind::ENUMERATION_TYPE:
          if(read_write == Direction::Write){
            m_int_writer.emplace_back(
                std::bind(writer_visitor<std::int32_t, std::uint32_t>, std::placeholders::_1, node.data()));
          }
          m_int_reader.emplace_back(
              std::bind(reader_visitor<std::int32_t, std::uint32_t>, std::placeholders::_1, node.data()));
          break;
        default:
          break;
        }
        break;
      case ddsfmu::Boolean:
        switch (node.type().kind())
        {
        case eprosima::xtypes::TypeKind::BOOLEAN_TYPE:
          if(read_write == Direction::Write){
            m_bool_writer.emplace_back(
                std::bind(writer_visitor<bool, bool>, std::placeholders::_1, node.data()));
          }
          m_bool_reader.emplace_back(
              std::bind(reader_visitor<bool, bool>, std::placeholders::_1, node.data()));
          break;
        default:
          break;
        }
        break;
      case ddsfmu::String:
        switch (node.type().kind())
        {
        case eprosima::xtypes::TypeKind::STRING_TYPE:
          if(read_write == Direction::Write){
            m_string_writer.emplace_back(
                std::bind(writer_visitor<std::string, std::string>, std::placeholders::_1, node.data()));
          }
          m_string_reader.emplace_back(
              std::bind(reader_visitor<std::string, std::string>, std::placeholders::_1, node.data()));
          break;
        case eprosima::xtypes::TypeKind::CHAR_8_TYPE:
          if(read_write == Direction::Write){
            m_string_writer.emplace_back(
                std::bind(writer_visitor<std::string, char>, std::placeholders::_1, node.data()));
          }
          m_string_reader.emplace_back(
              std::bind(reader_visitor<std::string, char>, std::placeholders::_1, node.data()));
          break;
        default:
          break;
        }
        break;
      default:
        break;
      }
    }

  });

}

void DataMapper::set_double(const std::int32_t value_ref, const double& value){
  m_real_writer.at(value_ref - m_real_offset)(value);
}

void DataMapper::get_double(const std::int32_t value_ref, double& value) const {
  m_real_reader.at(value_ref)(value);
}

void DataMapper::set_int(const std::int32_t value_ref, const std::int32_t& value){
  m_int_writer.at(value_ref - m_int_offset)(value);
}

void DataMapper::get_int(const std::int32_t value_ref, std::int32_t& value) const {
  m_int_reader.at(value_ref)(value);
}

void DataMapper::set_bool(const std::int32_t value_ref, const bool& value){
  m_bool_writer.at(value_ref - m_bool_offset)(value);
}

void DataMapper::get_bool(const std::int32_t value_ref, bool& value) const {
  m_bool_reader.at(value_ref)(value);
}

void DataMapper::set_string(const std::int32_t value_ref, const std::string& value){
  m_string_writer.at(value_ref - m_string_offset)(value);
}

void DataMapper::get_string(const std::int32_t value_ref, std::string& value) const {
  m_string_reader.at(value_ref)(value);
}
