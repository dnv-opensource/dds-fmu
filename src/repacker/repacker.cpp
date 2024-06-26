/*
  Copyright 2023, SINTEF Ocean
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <filesystem>
#include <iostream>

#include <args.hxx>
#include <zip/zip.h>

#include "SignalDistributor.hpp"
#include "auxiliaries.hpp"
#include "model-descriptor.hpp"
#include "dds-fmu/config.hpp"

namespace fs = std::filesystem;

namespace ddsfmu {
namespace detail {

/// @private
struct CommandsInfo {
  fs::path fmu_path, resources_path, binaries_path;
  fs::path model_description_tmpl;
  fs::path ddsfmu_mapping;
  fs::path ddsfmu_idl;
  fs::path zip_output;
  std::string default_zip;

  CommandsInfo(const std::string& in_path) {
    default_zip = "dds-fmu.fmu";
    fmu_path = fs::absolute(fs::path(in_path));
    resources_path = fmu_path / "resources";
    binaries_path = fmu_path / "binaries";
    model_description_tmpl = resources_path / "config" / "modelDescription.xml";
    ddsfmu_mapping = resources_path / "config" / "dds" / "ddsfmu_mapping.xml";
    ddsfmu_idl = resources_path / "config" / "idl" / "dds-fmu.idl";
    zip_output = fs::absolute(default_zip);
  }

  int prepare_for_zip(const fs::path& alternative_output) {
    if (!fs::is_directory(fmu_path)) {
      std::cerr << "ERROR: Directory does not exist: " << fmu_path << std::endl;
      return 1;
    }
    if (!fs::is_directory(resources_path)) {
      std::cerr << "ERROR: Directory does not exist: " << resources_path << std::endl;
      return 1;
    }
    if (!fs::is_directory(binaries_path)) {
      std::cerr << "ERROR: Directory does not exist: " << binaries_path << std::endl;
      return 1;
    }

    if (!alternative_output.empty()) { zip_output = alternative_output; }

    if (!zip_output.has_filename()) zip_output /= default_zip;

    if (!zip_output.has_extension()) zip_output.replace_extension("fmu");

    if (zip_output.has_extension() && zip_output.extension() != ".fmu") {
      zip_output.replace_extension("fmu");
      std::cout << "WARNING: Forcing .fmu extension for " << zip_output << std::endl;
    }

    return 0;
  }

  int prepare_for_generate() {
    if (!fs::is_directory(fmu_path)) {
      std::cerr << "ERROR: Directory does not exist: " << fmu_path << std::endl;
      return 1;
    }
    if (!fs::is_directory(resources_path)) {
      std::cerr << "ERROR: Directory does not exist: " << resources_path << std::endl;
      return 1;
    }


    if (!fs::exists(model_description_tmpl)) {
      std::cerr << "ERROR: ModelDescription template file does not exist: "
                << model_description_tmpl << std::endl;
      return 1;
    }
    if (!fs::exists(ddsfmu_mapping)) {
      std::cerr << "ERROR: DDS FMU mapping file does not exist: " << ddsfmu_mapping << std::endl;
      return 1;
    }
    if (!fs::exists(ddsfmu_idl)) {
      std::cerr << "ERROR: Main IDL file does not exist: " << ddsfmu_idl << std::endl;
      return 1;
    }

    return 0;
  }
};

}
}

int zip_fmu(const fs::path& fmu_root, const fs::path& out_file, bool verbose, bool force) {
  if (fs::exists(out_file)) {
    if (!force) {
      std::cerr << "ERROR: File already exists: " << out_file << std::endl
                << "Force overwriting file with -f flag" << std::endl;
      return 1;
    }

    if (verbose) std::cout << "INFO: Overwriting existing file" << std::endl;
  }

  if (verbose) {
    std::cout << "Packaging directory: " << fmu_root << std::endl;
    std::cout << "Writing to file: " << out_file << std::endl;
  }

  struct zip_t* zip = zip_open(out_file.string().c_str(), ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');

  if (zip == nullptr) {
    std::cerr << "ERROR: Unable to open zip file for writing: " << out_file << std::endl;
    return 1;
  }

  for (const fs::directory_entry& dir_entry : fs::recursive_directory_iterator(fmu_root)) {
    if (dir_entry.is_regular_file()) {
      if (verbose) {
        std::cout << "Adding: " << fs::relative(dir_entry, fmu_root).generic_string() << std::endl;
      }
      if (fs::relative(dir_entry, fmu_root) == fs::relative(out_file, fmu_root)) {
        std::cout << "Output file is within target directory, skipping: " << out_file << std::endl;
        continue;
      }
      auto relative_entry = fs::relative(dir_entry, fmu_root).generic_string();
      if (zip_entry_open(zip, relative_entry.c_str()) != 0) {
        std::cerr << "ERROR: Unable to open zip entry for writing: " << relative_entry << std::endl;
        return 1;
      } else if (zip_entry_fwrite(zip, fs::path(dir_entry).string().c_str()) != 0) {
        // TODO: figure out why permissions are lost
        // Confirm working on windows: What kind of slashes does it want?
        std::cerr << "ERROR: Unable to write zip entry: " << relative_entry << std::endl;
        return 1;
      } else if (zip_entry_close(zip) != 0) {
        std::cerr << "ERROR: Unable to close zip entry " << relative_entry << std::endl;
        return 1;
      }
    }
  }
  zip_close(zip);

  return 0;
}

int generate_xml(const ddsfmu::detail::CommandsInfo& info) {
  // load ddsfmu_mapping of signals to be mapped
  std::vector<char> buffer_0;
  rapidxml::xml_document<> signal_mapping;
  ddsfmu::config::load_ddsfmu_mapping(signal_mapping, info.ddsfmu_mapping, buffer_0);

  // load model description template configuration
  rapidxml::xml_document<> doc;
  std::vector<char> buffer_1;
  ddsfmu::config::load_template_xml(doc, info.model_description_tmpl, buffer_1);

  rapidxml::xml_node<>* root_node = doc.first_node("fmiModelDescription");
  rapidxml::xml_node<>* mv_node = doc.allocate_node(rapidxml::node_element, "ModelVariables");
  root_node->append_node(mv_node);

  auto distributor = ddsfmu::SignalDistributor();
  distributor.load_idls(info.resources_path); // load idl types into context

  auto mapper_ddsfmu = signal_mapping.first_node("ddsfmu");

  auto mapper_iterator = [&](ddsfmu::SignalDistributor::Cardinality cardinal) {
    std::string node_name;
    if (cardinal == ddsfmu::SignalDistributor::Cardinality::INPUT) {
      node_name = "fmu_in";
    } else {
      node_name = "fmu_out";
    }

    for (rapidxml::xml_node<>* fmu_node = mapper_ddsfmu->first_node(node_name.c_str()); fmu_node;
         fmu_node = fmu_node->next_sibling(node_name.c_str())) {
      auto topic = fmu_node->first_attribute("topic");
      auto type = fmu_node->first_attribute("type");
      if (!topic || !type) {
        std::cerr << "ERROR: <ddsfmu><" << node_name
                  << "> must specify attributes 'topic' and 'type'. Got: 'topic': "
                  << std::boolalpha << (topic != nullptr) << " and 'type': " << std::boolalpha
                  << (type != nullptr) << std::endl;
        throw std::runtime_error("Incomplete user data");
      }
      std::string topic_name(topic->value());
      std::string topic_type(type->value());

      bool do_key_filtering = false;

      if (cardinal == ddsfmu::SignalDistributor::Cardinality::OUTPUT) {
        auto key_filter = fmu_node->first_attribute("key_filter");
        if (key_filter) {
          std::istringstream(key_filter->value()) >> std::boolalpha >> do_key_filtering;
        }
      }

      if (!distributor.has_structure(topic_type)) {
        std::cerr << "ERROR: Got non-existing 'type': " << topic_type << std::endl;
        throw std::runtime_error("Unknown idl type");
      }

      //std::cout << "Topic: " << topic_name << " Type: " << topic_type << std::endl;
      distributor.add(topic_name, topic_type, cardinal);
      if (cardinal == ddsfmu::SignalDistributor::Cardinality::OUTPUT && do_key_filtering) {
        distributor.queue_for_key_parameter(topic_name, topic_type);
      }
    }
  };

  // out before in since this is assumed in module_structure_outputs further below!
  mapper_iterator(ddsfmu::SignalDistributor::Cardinality::OUTPUT);
  mapper_iterator(ddsfmu::SignalDistributor::Cardinality::INPUT);

  distributor.process_key_queue();

  // populate the model description file
  const auto& mapping = distributor.get_mapping();

  for (const auto& info : mapping) {
    //std::cout << "valRef: " << std::get<0>(info) << " causality: " << std::get<2>(info) << " name: " << std::get<1>(info) << std::endl;
    model_variable_generator(
      doc, mv_node, std::get<1>(info), std::get<2>(info), std::get<0>(info), std::get<3>(info));
  }

  ddsfmu::config::model_structure_outputs_generator(doc, root_node, distributor.outputs());

  // retrieve guid
  auto guid = ddsfmu::config::generate_uuid(ddsfmu::config::get_uuid_files(info.fmu_path, true));
  //,std::vector<std::string>{ddsfmu::config::print_xml(doc)}); // Uncomment and set get_uuid_files false in dds-fmu.cpp

  // update guid in model description
  auto guid_attr = root_node->first_attribute("guid");
  char* guid_val = doc.allocate_string(guid.c_str());
  auto new_guid = doc.allocate_attribute("guid", guid_val);
  root_node->insert_attribute(guid_attr, new_guid);
  root_node->remove_attribute(guid_attr);

  // write model description output to file
  ddsfmu::config::write_model_description(doc, info.fmu_path);

  return 0;
}


int main(int argc, const char* argv[]) {
  args::ArgumentParser combo_parser(
    "dds-fmu repacker tool", "Run 'repacker COMMAND --help' for more information on a command.");
  args::Group commands(combo_parser, "commands");
  args::Group arguments("arguments");
  args::GlobalOptions globals(combo_parser, arguments);
  args::HelpFlag help(arguments, "help", "Display this help menu. ", {'h', "help"});
  args::HelpFlag version(arguments, "version", "Display FMU version", {"version"});

  args::Command archiver(
    commands, "zip", "Create an FMU zip archive of PATH.", [&](args::Subparser& parser) {
      args::ValueFlag<std::string> output_file(
        parser, "FILENAME", "Output file (Default: 'dds-fmu.fmu')", {'o', "output"});
      args::Flag force(parser, "force", "Overwrite if output file exists", {'f', "force"});
      args::Flag verbose(parser, "verbose", "Verbose stream output", {'v', "verbose"});
      args::Positional<std::string> in_path(
        parser, "PATH", "Path to FMU root", args::Options::Required);
      parser.Parse();

      ddsfmu::detail::CommandsInfo info(args::get(in_path));
      info.prepare_for_zip(fs::path(args::get(output_file)));

      try {
        return zip_fmu(info.fmu_path, info.zip_output, verbose, force);
      } catch (const std::runtime_error& e) {
        std::cerr << "ERROR: Unable to zip FMU: " << e.what() << std::endl;
      }
      return 1;
    });

  args::Command generator(
    commands, "generate",
    "Generate ModelDescription XML using configuration files in FMU resources directory.",
    [&](args::Subparser& parser) {
      args::Flag verbose(parser, "verbose", "Verbose stream output", {'v', "verbose"});
      args::Positional<std::string> in_path(
        parser, "PATH", "Path to FMU root", args::Options::Required);
      parser.Parse();

      ddsfmu::detail::CommandsInfo info(args::get(in_path));
      info.prepare_for_generate();
      try {
        return generate_xml(info);
      } catch (const std::runtime_error& e) {
        std::cerr << "ERROR: Unable to generate model description for FMU: " << e.what()
                  << std::endl;
      }
      return 1;
    });


  args::Command create(
    commands, "create",
    "Repackaging of 'dds-fmu' with customised configuration files. It runs 'generate' and then "
    "'zip'.",
    [&](args::Subparser& parser) {
      args::ValueFlag<std::string> output_file(
        parser, "FILENAME", "Output file (Default: 'dds-fmu.fmu')", {'o', "output"});
      args::Flag force(parser, "force", "Overwrite if output file exists", {'f', "force"});
      args::Flag verbose(parser, "verbose", "Verbose stream output", {'v', "verbose"});
      args::Positional<std::string> in_path(
        parser, "PATH", "Path to FMU root", args::Options::Required);
      parser.Parse();

      ddsfmu::detail::CommandsInfo info(args::get(in_path));
      info.prepare_for_generate();
      info.prepare_for_zip(fs::path(args::get(output_file)));

      try {
        int res_1 = generate_xml(info);
        if (res_1 != 0) { return 1; }
        return zip_fmu(info.fmu_path, info.zip_output, verbose, force);
      } catch (const std::runtime_error& e) {
        std::cerr << "ERROR: Unable to create FMU: " << e.what() << std::endl;
      }
      return 1;
    });

  try {
    combo_parser.ParseCLI(argc, argv);
  } catch (const args::Help&) {
    if (args::get(version)) {
      std::cout << "dds-fmu version " << ddsfmu_version() << std::endl;
      return 0;
    }
    std::cout << combo_parser;
    return 0;
  } catch (const args::ParseError& e) {
    std::cerr << e.what() << std::endl;
    std::cerr << combo_parser;
    return 1;
  } catch (args::ValidationError e) {
    std::cerr << e.what() << std::endl;
    std::cerr << combo_parser;
    return 1;
  }

  return 0;
}
