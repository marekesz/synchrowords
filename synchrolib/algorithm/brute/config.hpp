#pragma once
#include <synchrolib/algorithm/config.hpp>
#include <external/json.hpp>
#include <string>
#include <utility>
#include <vector>

namespace synchrolib {

using json = nlohmann::json;

class BruteConfig : public AlgoConfig {
public:
  std::vector<std::pair<std::string, std::string>> get_substs(
      const json& config) const override {
    std::vector<std::pair<std::string, std::string>> ret;
    ret.push_back({"$BRUTE_DEF$", def(config)});
    ret.push_back({"$BRUTE_UNDEF$", undef()});
    return ret;
  }

private:
  static std::string def(const json& config) {
    std::string ret;
    ret += make_define("MAX_N", get_str_int(config, "max_n", "20"));

    return ret;
  }

  static std::string undef() { return make_undefine("MAX_N"); }
};

}  // namespace synchrolib