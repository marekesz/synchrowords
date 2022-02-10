#pragma once
#include <synchrolib/algorithm/config.hpp>
#include <external/json.hpp>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace synchrolib {

using json = nlohmann::json;

class ReduceConfig : public AlgoConfig {
public:
  std::vector<std::pair<std::string, std::string>> get_substs(
      const json& config) const override {
    std::vector<std::pair<std::string, std::string>> ret;
    ret.push_back({"$REDUCE_DEF$", def(config)});
    ret.push_back({"$REDUCE_UNDEF$", undef()});
    return ret;
  }

private:
  static std::string def(const json& config) {
    std::string ret;

    auto min_n = get_str_int(config, "min_n", "80");
    ret += make_define("MIN_N", min_n);

    auto list_size_threshold = get_str_int(config, "list_size_threshold", "AUT_N * 16");
    ret += make_define("LIST_SIZE_THRESHOLD", list_size_threshold);

    return ret;
  }

  static std::string undef() {
    return make_undefine("MIN_N") + make_undefine("LIST_SIZE_THRESHOLD");
  }
};

}  // namespace synchrolib