#pragma once

#include <dlfcn.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace jitlib {

using Path = std::filesystem::path;

namespace impl {

void replace(
    std::string& str, const std::unordered_map<std::string, std::string>& map) {
  for (const auto& pr : map) {
    std::string buf = "";
    size_t last = 0;
    size_t pos = 0;
    while ((pos = str.find(pr.first, pos)) != std::string::npos) {
      buf += str.substr(last, pos - last);
      buf += pr.second;
      pos += pr.first.size();
      last = pos;
    }
    buf += str.substr(last);
    str = buf;
  }
}

class NonCopyable {
public:
  NonCopyable(const NonCopyable&) = delete;
  NonCopyable& operator=(const NonCopyable&) = delete;
  NonCopyable() {}
};

class NonMovable {
public:
  NonMovable(NonMovable&&) = delete;
  NonMovable& operator=(NonMovable&&) = delete;
  NonMovable() {}
};

}  // namespace impl

class DirectoryChanger : public impl::NonCopyable, public impl::NonMovable {
public:
  DirectoryChanger(Path path) {
    old_dir_ = std::filesystem::current_path();
    std::filesystem::current_path(path);
  }

  ~DirectoryChanger() { std::filesystem::current_path(old_dir_); }

private:
  Path old_dir_;
};

class DefaultJitLibLogger : public impl::NonCopyable, public impl::NonMovable {
public:
  DefaultJitLibLogger() {}
  ~DefaultJitLibLogger() { std::cout << "\n"; }

  template<typename T>
  DefaultJitLibLogger& operator<<(const T& msg) {
    std::cout << msg;
    return *this;
  }
};

template<typename Logger = DefaultJitLibLogger>
class JitLib : public impl::NonCopyable {
public:

  class JitLibException : public std::exception {
  private:
    std::string msg;
  public:
    JitLibException(std::string message): msg(std::move(message)) {}
    const char * what() const noexcept override { return msg.c_str(); }
  };
  class SubstituteException : public JitLibException {
    using JitLibException::JitLibException;
  };
  class LoadException : public JitLibException {
    using JitLibException::JitLibException;
  };
  class RunException : public JitLibException {
    using JitLibException::JitLibException;
  };
  class CompilationException : public JitLibException {
    using JitLibException::JitLibException;
  };

  JitLib() : dl_(nullptr), func_(nullptr) {}
  ~JitLib() { cleanup(); }
  JitLib(JitLib&& jitlib)
      : dl_(jitlib.dl_), dir_(jitlib.dir_), func_(jitlib.func_) {
    jitlib.dl_ = nullptr;
    jitlib.dir_.clear();
    jitlib.func_ = nullptr;
  }
  JitLib& operator=(JitLib&& jitlib) {
    dl_ = jitlib.dl_;
    dir_ = jitlib.dir_;
    func_ = jitlib.func_;
    jitlib.dl_ = nullptr;
    jitlib.dir_.clear();
    jitlib.func_ = nullptr;
    return *this;
  }

  JitLib& substitute(
      std::vector<std::pair<Path, Path>> input, Path out_dir, const std::unordered_map<std::string, std::string>& map) {
    dir_ = out_dir;

    if (!std::filesystem::is_directory(dir_)) {
      throw SubstituteException(dir_.string() + " is not a directory");
    }
    for (const auto& entry : std::filesystem::directory_iterator(dir_)) {
      try {
        std::filesystem::remove_all(entry.path());
      } catch (const std::filesystem::filesystem_error& error) {
        Logger() << "Could not remove directory:\n" << error.what();
      }
    }

    Logger() << "Copying files";
    for (auto [from, to] : input) {
      try {
        const auto copy_options =
            std::filesystem::copy_options::overwrite_existing |
            std::filesystem::copy_options::recursive;
        std::filesystem::copy(from, dir_ / to, copy_options);
      } catch (const std::filesystem::filesystem_error& error) {
        Logger() << "Could not copy " << from.string() << ":\n" << error.what();
        dir_ = Path();
        throw SubstituteException(error.what());
      }
    }

    for (auto& entry : std::filesystem::recursive_directory_iterator(dir_)) {
      if (!entry.is_regular_file()) {
        continue;
      }

      std::ifstream istream(entry.path());
      std::stringstream buffer;
      buffer << istream.rdbuf();
      auto str = buffer.str();
      impl::replace(str, map);
      istream.close();

      std::ofstream ostream(entry.path(), std::ofstream::trunc);
      ostream << str;
      ostream.close();
    }

    return *this;
  }

  JitLib& compile(const std::string& make_args="jit", bool show_compilation_output=false) {
    if (dir_.empty()) {
      Logger() << "Can not compile library, directory not ready";
      throw CompilationException("directory empty");
    }

    DirectoryChanger cwd(dir_);
    Logger() << "Compiling";
    std::string cmd = "make " + make_args + " -j16"; // TODO: -jX global config parameter
    if (!show_compilation_output) {
      cmd += " >/dev/null";
    }

    std::cout << std::flush; std::cerr << std::flush;
    int ret = std::system(cmd.c_str());
    std::cout << std::flush; std::cerr << std::flush;

    if (ret != 0) {
      Logger() << "Compilation failed";
      throw CompilationException("compilation failed");
    }

    return *this;
  }

  JitLib& set_dir_path(Path p) {
    dir_ = p;
    return *this;
  }

  JitLib& load(Path lib) {
    if (dir_.empty()) {
      Logger() << "Can not load library, directory not ready";
      throw LoadException("directory empty");
    }

    unload_if_loaded();
    Logger() << "Loading library " << lib;
    dlerror();  // clear errors
    auto lib_path = dir_ / lib;
    dl_ = dlopen(lib_path.c_str(), RTLD_NOW);
    auto err = dlerror();
    if (err || !dl_) {
      Logger() << "Error while loading " << lib << ":\n"
                << err;
      throw LoadException("loading library failed");
    }

    return *this;
  }

  template <typename... Ts, typename... Args>
  JitLib& run(std::string func_name, Args&&... args) {
    Logger() << "Finding function " << func_name;
    dlerror();  // clear errors
    *(void**)&func_ = dlsym(dl_, func_name.c_str());
    auto err = dlerror();
    if (err || !func_) {
      Logger() << "Could not find function " << func_name << ":\n"
          << err;
      throw RunException("function not found");
    }

    Logger() << "Running function";
    reinterpret_cast<void (*)(Ts...)>(func_)(std::forward<Args>(args)...);
    return *this;
  }

private:
  void cleanup() {
    unload_if_loaded();
  }

  void unload_if_loaded() {
    if (dl_) {
      Logger() << "Unloading library";
      dlerror();  // clear errors
      if (dlclose(dl_)) {
        Logger() << "Failed to unload library:\n" << dlerror();
      }
    }
  }

  void* dl_;
  Path dir_;
  void* func_;
};

}  // namespace jitlib
