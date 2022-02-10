#pragma once
#include <synchrolib/utils/logger.hpp>
#include <synchrolib/algorithm/algorithm.hpp>
#include <external/json.hpp>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>


class IO {
public:
  using json = nlohmann::json;
  using AlgoResult = synchrolib::AlgoResult;
  using Path = std::filesystem::path;

  static json read_config(Path path) {
    std::ifstream ss(path);
    json ret;
    try {
      ss >> ret;
    } catch (nlohmann::detail::exception& ex) {
      Logger::error() << ex.what();
      std::exit(2);
    }

    Logger::verbose() << "Config:\n" << ret.dump(4);
    return ret;
  }

  struct EncodedAutomaton {
    uint N, K;
    std::string str;

    void validate() {
      std::stringstream ss(str);
      for (uint i = 0; i < N * K; ++i) {
        uint cur;
        if (!(ss >> cur)) {
          Logger::error() << "Expected " << N * K << " integers, found " << i;
          std::exit(3);
        }
        if (cur >= N) {
          Logger::error() << "Expected integer in range [0, " << N - 1 << "], found " << cur;
          std::exit(3);
        }
      }
    }
  };

  // TODO: error handling (no file or bad format)
  static std::vector<EncodedAutomaton> read_automata(Path path) {
    std::vector<EncodedAutomaton> ret;
    std::ifstream ss(path);

    std::string line;
    int line_num;
    while (std::getline(ss, line)) {
      uint N, K;
      std::stringstream nkss(line);
      if (!(nkss >> N >> K)) {
        continue;
      }
      if (N == 0 || K == 0) {
        Logger::error() << "N and K must be greater than 0";
        std::exit(3);
      }
      std::getline(ss, line);
      ret.push_back(EncodedAutomaton{N, K, line});
      ret.back().validate();
    }

    Logger::info() << "Read " << ret.size() << " automata";
    return ret;
  }

  static void set_output(std::ofstream stream) {
    output = std::move(stream);
  }

  static void push_result(const AlgoResult& result) {
    if (!output) {
      if (result.word) {
        Logger::info() << "Found synchronizing word of length "
                        << result.word->size()
                        << " (use the -o flag to save it)";
      }
      return;
    }

    if (result.non_synchro) {
      *output << "NON SYNCHRO\n";
      output->flush();
      return;
    }

    *output << "[" << result.mlsw_lower_bound << ", "
            << result.mlsw_upper_bound << "]";

    if (result.word) {
      Logger::info() << "Saving synchronizing word of length "
                      << result.word->size();

      *output << " {";
      for (size_t i = 0; i < result.word->size(); ++i) {
        if (i != 0) {
          *output << " ";
        }
        *output << (*result.word)[i];
      }
      *output << "}";
    }

    *output << "\n";
    output->flush();
  }

private:
  using Logger = synchrolib::Logger;

  inline static std::optional<std::ofstream> output;

  IO() {}
};
