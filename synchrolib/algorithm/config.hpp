#pragma once
#include <external/json.hpp>
#include <string>
#include <utility>
#include <vector>

namespace synchrolib {

using json = nlohmann::json;

std::string add_parentheses(std::string str) { return "(" + str + ")"; }

std::string make_define(std::string name, std::string value = "") {
  return "#define " + name + (value.empty() ? "" : " " + add_parentheses(value)) + "\n";
}
std::string make_define(std::string name, int64_t value) {
  return "#define " + name + " " + add_parentheses(std::to_string(value)) + "\n";
}
std::string make_define(std::string name, bool value) {
  return "#define " + name + " " + add_parentheses((value ? "true" : "false")) + "\n";
}

std::string make_undefine(std::string name) { return "#undef " + name + "\n"; }

// TODO: code duplication
std::string get_str_int(const json& json, const std::string& key, std::string default_value) {
  if (!json.contains(key)) {
    return default_value;
  }
  auto val = json[key];
  if (val.is_string()) {
    return val.get<std::string>();
  }
  return std::to_string(val.get<long long>());
}
std::string get_str_bool(const json& json, const std::string& key, std::string default_value) {
  if (!json.contains(key)) {
    return default_value;
  }
  auto val = json[key];
  if (val.is_string()) {
    return val.get<std::string>();
  }
  return val.get<bool>() ? "true" : "false";
}
std::string get_str_float(const json& json, const std::string& key, std::string default_value) {
  if (!json.contains(key)) {
    return default_value;
  }
  auto val = json[key];
  if (val.is_string()) {
    return val.get<std::string>();
  }
  return std::to_string(val.get<long double>());
}

class AlgoConfig {
public:
  virtual std::vector<std::pair<std::string, std::string>> get_substs(
      const json& config) const = 0;
};

}  // namespace synchrolib