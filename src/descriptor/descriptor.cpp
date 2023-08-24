/**
   @brief Generate ModelDescription XML

   Creates the complete modelDescription.xml by using configuration files in a given directory.

*/
#include <filesystem>
#include <iostream>
#include <args.hxx>

#include "model-descriptor.hpp"
#include "idl-loader.hpp"
#include "auxiliaries.hpp"
#include "SignalDistributor.hpp"

int main(int argc, const char* argv[])
{
  namespace fs = std::filesystem;
  args::ArgumentParser parser("Generate ModelDescription XML using configuration files in FMU resource directory. It is intended for repackaging of 'dds-fmu' with customised configuration files.", "This tool is part of the FMU 'dds-fmu'.");
  args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
  args::Flag verbose(parser, "verbose", "Verbose stream output", {'v', "verbose"});
  args::Positional<std::string> in_path(parser, "path", "Path to fmu root", args::Options::Required);
  try
  {
    parser.ParseCLI(argc, argv);
  }
  catch (const args::Help&)
  {
    std::cout << parser;
    return 0;
  }
  catch (const args::ParseError& e)
  {
    std::cerr << e.what() << std::endl;
    std::cerr << parser;
    return 1;
  }
  catch (args::ValidationError e)
  {
    std::cerr << e.what() << std::endl;
    std::cerr << parser;
    return 1;
  }
  auto fmu_path = fs::absolute(fs::path(args::get(in_path)));
  auto resources_path = fmu_path / "resources";

  if (!fs::is_directory(fmu_path)){
    std::cerr << "Directory does not exist: " << fmu_path << std::endl;
    return 1;
  } else if (!fs::is_directory(resources_path)){
    std::cerr << "Directory does not exist: " << resources_path << std::endl;
    return 1;
  }

  auto modelDescription_template = resources_path / "config" / "modelDescription.xml";
  auto ddsfmu_mapping = resources_path / "config" / "dds" / "ddsfmu_mapping.xml";
  auto ddsfmu_idl = resources_path / "config" / "idl" / "dds-fmu.idl";

  if (!fs::exists(modelDescription_template)){
    std::cerr << "ModelDescription template file does not exist: " << modelDescription_template << std::endl;
    return 1;
  } else if(!fs::exists(ddsfmu_mapping)){
    std::cerr << "DDS FMU mapping file does not exist: " << ddsfmu_mapping << std::endl;
    return 1;
  } else if(!fs::exists(ddsfmu_idl)){
    std::cerr << "Main IDL file does not exist: " << ddsfmu_idl << std::endl;
    return 1;
  }

  // load ddsfmu_mapping of signals to be mapped
  std::vector<char> buffer_0;
  rapidxml::xml_document<> signal_mapping;
  load_ddsfmu_mapping(signal_mapping, ddsfmu_mapping, buffer_0);

  // load model description template configuration
  rapidxml::xml_document<> doc;
  std::vector<char> buffer_1;
  load_template_xml(doc, modelDescription_template, buffer_1);

  rapidxml::xml_node<> *root_node = doc.first_node("fmiModelDescription");
  rapidxml::xml_node<>* mv_node = doc.allocate_node(rapidxml::node_element, "ModelVariables");
  root_node->append_node(mv_node);

  auto distributor = SignalDistributor();
  distributor.load_idls(resources_path);   // load idl types into context

  auto mapper_ddsfmu = signal_mapping.first_node("ddsfmu");

  auto mapper_iterator = [&](bool is_in_not_out) {

    std::string node_name;
    if(is_in_not_out) node_name = "fmu_in";
    else node_name = "fmu_out";

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

      if(!distributor.has_structure(topic_type)){
        std::cerr << "Got non-existing 'type': " << topic_type << std::endl;
        throw std::runtime_error("Unknown idl type");
      }

      //std::cout << "Topic: " << topic_name << " Type: " << topic_type << std::endl;
      distributor.add(topic_name, topic_type, is_in_not_out);
    }
  };

  // out before in since this is assumed in module_structure_outputs further below!
  mapper_iterator(false); // out
  mapper_iterator(true); // in

  // populate the model description file
  const auto& mapping = distributor.get_mapping();

  for (const auto& info : mapping) {
    std::cout << "valRef: " << std::get<0>(info) << " causality: " << std::get<2>(info) << " name: " << std::get<1>(info) << std::endl;
    model_variable_generator(doc, mv_node, std::get<1>(info), std::get<2>(info), std::get<0>(info), std::get<3>(info));
  }

  model_structure_outputs_generator(doc, root_node, distributor.outputs());

  //model_variable_generator(doc, mv_node, "y", "output", 0, ddsfmu::Real);

  // For each input
  //model_variable_generator(doc, mv_node, "x", "input", 1, ddsfmu::Real);

  // Count outputs and call this
  //model_structure_outputs_generator(doc, root_node, 1);

  // retrieve guid
  auto guid = generate_uuid(get_uuid_files(fmu_path, true), std::vector<std::string>{print_xml(doc)});

  // update guid in model description
  auto guid_attr = root_node->first_attribute("guid");
  char* guid_val = doc.allocate_string(guid.c_str());
  auto new_guid = doc.allocate_attribute("guid", guid_val);
  root_node->insert_attribute(guid_attr, new_guid);
  root_node->remove_attribute(guid_attr);

  // write model description output to file
  write_model_description(doc, fmu_path);

  return 0;
}
