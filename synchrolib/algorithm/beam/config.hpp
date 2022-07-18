#pragma once
#include <synchrolib/algorithm/config.hpp>
#include <external/json.hpp>
#include <string>
#include <utility>
#include <vector>

namespace synchrolib {

using json = nlohmann::json;

class BeamConfig : public AlgoConfig {
public:
  std::vector<std::pair<std::string, std::string>> get_substs(
      const json& config) const override {
    std::vector<std::pair<std::string, std::string>> ret;
    ret.push_back({"$BEAM_DEF$", def(config)});
    ret.push_back({"$BEAM_UNDEF$", undef()});
    return ret;
  }

private:
  static std::string def(const json& config) {
    std::string ret;
    ret += make_define("BEAM_SIZE", get_str_int(config, "beam_size", "std::log2(AUT_N)"));

    ret += make_define("DYNAMIC", get_str_bool(config, "dynamic", "false"));
    ret += make_define("MIN_BEAM_SIZE", get_str_int(config, "min_beam_size", "AUT_N * std::log2(AUT_N)"));
    ret += make_define("MAX_BEAM_SIZE", get_str_int(config, "max_beam_size", "AUT_N * AUT_N * std::log2(AUT_N)"));
    ret += make_define("BEAM_EXACT_RATIO", get_str_float(config, "beam_exact_ratio", "0.01"));

    ret += make_define("MAX_ITER", get_str_int(config, "max_iter", "-1"));

    auto presort = config.value("presort", "none");
    if (presort == "none") {
      ret += make_define("PRESORT_NONE");
    } else if (presort == "indeg") {
      ret += make_define("PRESORT_INDEG");
    } else {
      throw std::runtime_error("presort value not in [none, indeg]");
    }

    return ret;
  }

  static std::string undef() {
    return make_undefine("BEAM_SIZE") + make_undefine("PRESORT_NONE") +
        make_undefine("PRESORT_INDEG") + make_undefine("MAX_ITER");
  }
};

}  // namespace synchrolib