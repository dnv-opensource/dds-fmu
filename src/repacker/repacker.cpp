#include <iostream>
#include <filesystem>

#include <args.hxx>
#include <zip/zip.h>

namespace fs = std::filesystem;

int zip_fmu(const fs::path& fmu_root, const fs::path& out_file, bool verbose, bool force){

  if (fs::exists(out_file)){

    if (!force){
      std::cerr << "File already exists: " << out_file << std::endl
                << "Force overwriting file with -f flag" << std::endl;
      return 1;
    }

    if(verbose) std::cout << "Info: Overwriting existing file" << std::endl;
  }

  if (verbose){
    std::cout << "Packaging directory: " << fmu_root << std::endl;
    std::cout << "Writing to file: " << out_file << std::endl;
  }

  struct zip_t *zip = zip_open(out_file.c_str(), ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');

  if (zip == nullptr){
    std::cerr << "Unable to open zip file for writing: " << out_file << std::endl;
    return 1;
  }

  for (const fs::directory_entry& dir_entry : fs::recursive_directory_iterator(fmu_root))
  {
    if (dir_entry.is_regular_file()){
      if (verbose){
        std::cout << "Adding: " << fs::relative(dir_entry, fmu_root).generic_string() << std::endl;
      }
      auto relative_entry = fs::relative(dir_entry, fmu_root).generic_string();
      if (zip_entry_open(zip, relative_entry.c_str()) != 0){
        std::cerr << "Unable to open zip entry for writing: " << relative_entry << std::endl;
        return 1;
      } else if (zip_entry_fwrite(zip, fs::path(dir_entry).c_str()) != 0){
        // TODO: figure out why permissions are lost
        // Confirm working on windows: What kind of slashes does it want?
        std::cerr << "Unable to write zip entry: " << relative_entry << std::endl;
        return 1;
      } else if (zip_entry_close(zip) != 0){
        std::cerr << "Unable to close zip entry " << relative_entry << std::endl;
        return 1;
      }
    }
  }
  zip_close(zip);

  return 0;
}

int main(int argc, const char * argv[]){

  args::ArgumentParser parser("Creates an FMU archive of the specified path. It is intended for repackaging of 'dds-fmu' with customised configuration files.", "This tool is part of the FMU 'dds-fmu'.");
  args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
  args::Flag force(parser, "force", "Overwrite if output file exists", {'f', "force"});
  args::Flag verbose(parser, "verbose", "Verbose stream output", {'v', "verbose"});
  args::ValueFlag<std::string> output_file(parser, "filename", "Output file", {'o', "output"});
  args::Positional<std::string> in_path(parser, "path", "Path to repackage", args::Options::Required);
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
  auto fmu_path = fs::path(args::get(in_path));
  auto binaries_path = fmu_path / "binaries";
  auto resources_path = fmu_path / "resources";
  std::string default_filename("dds-fmu.fmu");
  auto out_file = std::filesystem::absolute(default_filename);
  if (!fs::is_directory(fmu_path)){
    std::cerr << "Directory does not exist: " << fmu_path << std::endl;
    return 1;
  } else if (!fs::is_directory(binaries_path)){
    std::cerr << "Directory does not exist: " << binaries_path << std::endl;
    return 1;
  } else if (!fs::is_directory(resources_path)){
    std::cerr << "Directory does not exist: " << resources_path << std::endl;
    return 1;
  }
  auto fmu_root = fs::absolute(fmu_path);

  if (output_file){

    out_file = fs::absolute(args::get(output_file));

    if (!out_file.has_filename())
      out_file /= default_filename;

    if (!out_file.has_extension())
      out_file.replace_extension("fmu");

    if (out_file.has_extension() && out_file.extension() != ".fmu"){
      out_file.replace_extension("fmu");
      std::cout << "Warning: Forcing .fmu extension for " << out_file << std::endl;
    }
  }

  return zip_fmu(fmu_root, out_file, verbose, force);
}
