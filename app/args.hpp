#pragma once
#include <external/cxxopts.hpp>
#include <synchrolib/utils/logger.hpp>
#include <synchrolib/utils/general.hpp>
#include <optional>
#include <string>
#include <filesystem>

class CmdArgs {
public:
  using Path = std::filesystem::path;

  Path input_path;
  Path config_path;
  std::optional<Path> output_path;
  bool verbose;
  bool quiet;
  bool debug;
  bool cont;
  std::string build_suffix;

  CmdArgs() : verbose(false) {}

  CmdArgs(const cxxopts::ParseResult& result) {
    input_path = Path(*get_value<std::string>(result, "file", true));
    config_path = Path(*get_value<std::string>(result, "config", true));

    auto output = get_value<std::string>(result, "output", false);
    if (output) {
      output_path = Path(*output);
    }

    build_suffix = result["build-suffix"].as<std::string>();

    verbose = result.count("verbose");
    quiet = result.count("quiet");
    debug = result.count("debug");
    cont = result.count("continue");

    if (static_cast<int>(verbose) + static_cast<int>(quiet) + static_cast<int>(debug) > 1) {
      Logger::error() << "Only one of [--verbose, --quiet, --debug] can be enabled at one time";
      std::exit(1);
    }

    if (cont && !output_path) {
      Logger::error() << "--continue can be enabled only if an output file is specified";
      std::exit(1);
    }
  }

  static CmdArgs parse_arguments(int argc, char** argv) {
    cxxopts::Options options(
        "synchro", "Find synchronizing word of an automaton");
    options.add_options()(
        "f,file", "Path to the input file", cxxopts::value<std::string>())(
        "c,config", "Path to the config file", cxxopts::value<std::string>())(
        "o,output", "Path to the output file", cxxopts::value<std::string>())(
        "b,build-suffix", "Suffix of the build folder", cxxopts::value<std::string>()->default_value(""))(
        "continue", "Do not overwrite the output file and run algorithms only for remaining automata")(
        "v,verbose", "Verbose output")(
        "q,quiet", "Quiet output (only warnings and errors)")(
        "d,debug", "Debug output (all messages and timers)")(
        "h,help", "Print usage");

    try {
      auto result = options.parse(argc, argv);
      if (result.count("help")) {
        std::cout << options.help();
        std::exit(0);
      }

      return CmdArgs(result);

    } catch (cxxopts::OptionException& ex) {
      Logger::error() << ex.what();
      std::exit(1);
    }
  }

private:
  using Logger = synchrolib::Logger;

  template<typename T>
  static std::optional<T> get_value(const cxxopts::ParseResult& result, const std::string& key, bool required=true) {
    if (!result.count(key)) {
      if (required) {
        Logger::error() << "Required argument ‘" << key << "’ not found";
        std::exit(1);
      }
      return std::nullopt;
    }
    return result[key].as<T>();
  }
};
