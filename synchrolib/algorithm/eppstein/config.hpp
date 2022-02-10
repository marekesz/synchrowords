#pragma once
#include <synchrolib/algorithm/config.hpp>
#include <external/json.hpp>
#include <string>
#include <utility>
#include <vector>

namespace synchrolib {

using json = nlohmann::json;

class EppsteinConfig : public AlgoConfig {
public:
  std::vector<std::pair<std::string, std::string>> get_substs(
      const json& config) const override {
    std::vector<std::pair<std::string, std::string>> ret;
    ret.push_back({"$EPPSTEIN_DEF$", def(config)});
    ret.push_back({"$EPPSTEIN_UNDEF$", undef()});
    return ret;
  }

private:
  static std::string def(const json& config) {
    std::string ret;
    auto transition_tables = get_str_bool(config, "transition_tables", "false");
    ret += make_define("TRANSITION_TABLES", transition_tables);

    auto find_word = get_str_bool(config, "find_word", "false");
    ret += make_define("FIND_WORD", find_word);

    return ret;
  }

  static std::string undef() {
    return make_undefine("TRANSITION_TABLES") + make_undefine("FIND_WORD");
  }
};

}  // namespace synchrolib