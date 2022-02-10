#pragma once
#include <synchrolib/algorithm/config.hpp>
#include <external/json.hpp>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace synchrolib {

using json = nlohmann::json;

class ExactConfig : public AlgoConfig {
public:
  std::vector<std::pair<std::string, std::string>> get_substs(
      const json& config) const override {
    return {
      {"$EXACT_DEF$", def(config)},
      {"$EXACT_UNDEF$", undef()}
    };
  }

private:
  static std::string def(const json& config) {
    std::string ret;

    auto dfs_shortcut = get_str_bool(config, "dfs_shortcut", "true");
    ret += make_define("DFS_SHORTCUT", dfs_shortcut);

    auto dfs = get_str_bool(config, "dfs", "true");
    ret += make_define("DFS", dfs);

    auto strict_memory_limit = get_str_bool(config, "strict_memory_limit", "false");
    ret += make_define("STRICT_MEMORY_LIMIT", strict_memory_limit);

    auto max_memory = get_str_int(config, "max_memory_mb", "2048");
    ret += make_define("MAX_MEMORY", max_memory);

    auto dfs_min_list_size = get_str_int(config, "dfs_min_list_size", "10000");
    ret += make_define("DFS_MIN_LIST_SIZE", dfs_min_list_size);

    auto bfs_small_list_size = get_str_int(config, "bfs_small_list_size", "AUT_N * 16");
    ret += make_define("BFS_SMALL_LIST_SIZE", bfs_small_list_size);

    return ret;
  }

  static std::string undef() {
    return
        make_undefine("DFS_SHORTCUT") + make_undefine("MAX_MEMORY") +
        make_undefine("DFS_MIN_LIST_SIZE") + make_undefine("BFS_SMALL_LIST_SIZE") +
        make_undefine("DFS") + make_undefine("STRICT_MEMORY_LIMIT");
  }
};

}  // namespace synchrolib