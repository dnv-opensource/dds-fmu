#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>
#include <vector>

#include <args.hxx>

#include "auxiliaries.hpp"

int main(int argc, const char* argv[]){

  args::ArgumentParser parser("Evaluates the UUID for the FMU and outputs to output stream. This is temporary.", "This tool is part of the FMU 'dds-fmu'.");
  args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
  args::Positional<std::string> in_path(parser, "path", "Path to FMU base path", args::Options::Required);
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
  namespace fs = std::filesystem;
  auto uuid_files = get_uuid_files(fs::absolute(args::get(in_path)));
  std::cout << generate_uuid(uuid_files) << std::endl;

  return 0;
}
