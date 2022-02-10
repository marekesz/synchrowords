#include <synchrolib/utils/logger.hpp>
#include <app/args.hpp>
#include <app/io.hpp>
#include <app/jit.hpp>
#include <string>


using Logger = synchrolib::Logger;

int main(int argc, char** argv) {
  auto args = CmdArgs::parse_arguments(argc, argv);

  if (args.verbose) {
    Logger::set_log_level(Logger::LogLevel::VERBOSE);
  } else if (args.quiet) {
    Logger::set_log_level(Logger::LogLevel::WARNING);
  } else if (args.debug) {
    Logger::set_log_level(Logger::LogLevel::DEBUG);
  }

  auto auts_encoded = IO::read_automata(args.input_path);
  auto config = IO::read_config(args.config_path);

  if (args.output_path) {
    IO::set_output(std::ofstream(*args.output_path));
  }

  for (const auto& aut : auts_encoded) {
    Jit::AlgoResult result;
    uint cur_n = aut.N;
    uint cur_k = aut.K;
    std::string cur_aut = aut.str;

    while (!Jit::run(config, cur_n, cur_k, cur_aut, result)) {
      if (result.reduce && !result.reduce->done) {
        cur_n = result.reduce->aut.N;
        cur_k = result.reduce->aut.K;
        cur_aut = result.reduce->aut.code();

        result.reduce->done = true;
      }
    }

    if (result.non_synchro) {
      Logger::info() << "NON SYNCHRO";
    } else {
      Logger::info() << "Minimum synchronizing word length: ["
                      << result.mlsw_lower_bound << ", "
                      << result.mlsw_upper_bound << "]";
    }
    IO::push_result(result);
  }

  return 0;
}
