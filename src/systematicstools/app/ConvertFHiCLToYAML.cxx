#include "systematicstools/utility/ConfigConverter.hh"

#include "cetlib/filepath_maker.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

namespace {

struct CliOptions {
  std::string input_fcl;
  std::string output_yaml;
};

void SayUsage(char const *argv0) {
  std::cout
      << "[USAGE]: " << argv0 << " -c <config.fcl> -o <output.yaml>\n\n"
      << "  Converts a FHiCL configuration file to YAML.\n\n"
      << "  Options:\n"
      << "    -c <config.fcl>   Input FHiCL file. Resolved using FHICL_FILE_PATH.\n"
      << "    -o <output.yaml>  Output YAML file path.\n"
      << "    -?|--help         Show this message.\n";
}

CliOptions ParseCli(int argc, char const *argv[]) {
  CliOptions opts;

  for (int idx = 1; idx < argc; ++idx) {
    std::string arg = argv[idx];
    if (arg == "-?" || arg == "--help") {
      SayUsage(argv[0]);
      std::exit(0);
    }
    if (arg == "-c") {
      if (idx + 1 >= argc) {
        throw std::runtime_error("Missing value for -c.");
      }
      opts.input_fcl = argv[++idx];
      continue;
    }
    if (arg == "-o") {
      if (idx + 1 >= argc) {
        throw std::runtime_error("Missing value for -o.");
      }
      opts.output_yaml = argv[++idx];
      continue;
    }

    throw std::runtime_error("Unknown option: " + arg);
  }

  if (opts.input_fcl.empty()) {
    throw std::runtime_error("Input FHiCL file is required. Use -c <config.fcl>.");
  }
  if (opts.output_yaml.empty()) {
    throw std::runtime_error("Output YAML path is required. Use -o <output.yaml>.");
  }

  return opts;
}

} // namespace

int main(int argc, char const *argv[]) {
  try {
    CliOptions opts = ParseCli(argc, argv);

    char const *fhicl_path_env = std::getenv("FHICL_FILE_PATH");
    std::string fhicl_paths = fhicl_path_env ? fhicl_path_env : "";
    cet::filepath_first_absolute_or_lookup_with_dot lookup_policy{fhicl_paths};
    fhicl::ParameterSet pset =
        fhicl::ParameterSet::make(opts.input_fcl, lookup_policy);

    YAML::Node yaml = systtools::FHiCLToYAML(pset);

    std::ofstream output(opts.output_yaml);
    if (!output.is_open()) {
      std::cerr << "[ERROR]: Failed to open output file '" << opts.output_yaml
                << "'." << std::endl;
      return 2;
    }

    output << YAML::Dump(yaml) << std::endl;
    if (!output.good()) {
      std::cerr << "[ERROR]: Failed while writing YAML to '" << opts.output_yaml
                << "'." << std::endl;
      return 3;
    }

    std::cout << "Wrote YAML to " << opts.output_yaml << std::endl;
    return 0;
  } catch (std::exception const &ex) {
    std::cerr << "[ERROR]: " << ex.what() << std::endl;
    SayUsage(argv[0]);
    return 1;
  }
}