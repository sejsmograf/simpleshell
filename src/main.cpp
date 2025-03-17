#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

std::set<std::string> all_cmds{"exit", "echo", "type", "pwd", "cd"};

bool is_executable(const std::string &path) {
  return !access(path.c_str(), X_OK) && !std::filesystem::is_directory(path);
}

std::filesystem::path get_current_dir() {
  return std::filesystem::current_path();
}

std::vector<std::filesystem::path> getpath() {
  std::vector<std::filesystem::path> path;
  std::string path_string = getenv("PATH");
  std::stringstream ss(path_string);

  while (std::getline(ss, path_string, ':')) {
    path.push_back(std::filesystem::path(path_string));
  }

  return path;
}

std::vector<std::string> getargv(const std::string &args_string) {
  std::vector<std::string> args;
  std::string arg = "";
  bool inside_quotes = false;

  for (size_t i = 0; i < args_string.size(); i++) {
    char at = args_string[i];

    if (at == '\'') {
      if (inside_quotes && !arg.empty()) {
        if (i + 1 < args_string.size() && args_string[i + 1] == '\'') {
          i++;
        } else {
          args.push_back(arg);
          arg.clear();
          inside_quotes = false;
        }
      } else {
        inside_quotes = true;
      }
    } else if (at == ' ' && !inside_quotes) {
      if (!arg.empty()) {
        args.push_back(arg);
        arg.clear();
      }
    } else {
      arg += at;
    }
  }

  if (inside_quotes) {
    std::cout << "args: Quotes not closed";
    return {};
  }

  if (!arg.empty()) {
    args.push_back(arg);
  }

  return args;
}

void exit(const std::string &args_string) { exit(0); }

void echo(const std::string &args_string) {
  auto args = getargv(args_string);

  for (size_t i = 0; i < args.size(); i++) {
    if (i != 0)
      std::cout << " ";
    std::cout << args[i];
  }
  std::cout << std::endl;
}

void type(const std::string &args_string) {
  auto args = getargv(args_string);
  if (args.size() < 1) {
    std::cout << std::endl;
    return;
  }

  auto first = args[0];

  if (all_cmds.contains(first)) {
    std::cout << first << " is a shell builtin" << std::endl;
  } else {
    auto path = getpath();
    for (const auto &p : path) {
      auto joined = p / first;
      if (std::filesystem::exists(joined)) {
        std::cout << first << " is " << joined.string() << std::endl;
        return;
      }
    }
    std::cout << first << ": not found" << std::endl;
  }
}

void pwd() {
  auto current = get_current_dir();
  std::cout << current.string() << std::endl;
}

void change_working_dir(const std::filesystem::path &path) {
  if (chdir(path.c_str()) != 0) {
    std::cout << "cd: " << path.string() << ": " << "No such file or directory"
              << std::endl;
  };
}

void cd(const std::string &args_string) {
  auto args = getargv(args_string);
  if (args.size() < 1) {
    std::filesystem::path home = getenv("HOME");
    change_working_dir(home);
    return;
  }

  auto path = std::filesystem::path(args[0]);

  if (path.string() == "~") {
    std::filesystem::path home = getenv("HOME");
    change_working_dir(home);

    return;
  }

  change_working_dir(path);
}

void handle_shell_builtin(const std::string &cmd,
                          const std::string &args_string) {
  if (cmd == "exit") {
    exit(args_string);
  } else if (cmd == "echo") {
    echo(args_string);
  } else if (cmd == "type") {
    type(args_string);
  } else if (cmd == "pwd") {
    pwd();
  } else if (cmd == "cd") {
    cd(args_string);
  }
}

std::filesystem::path find_executable(const std::string &cmd) {
  auto path = getpath();
  std::filesystem::path executable("");

  for (const auto &dir : path) {
    auto joined = dir / cmd;
    if (is_executable(joined)) {
      executable = joined;
      break;
    }
  }

  return executable;
}

void handle_external_cmd(const std::string &cmd,
                         const std::string &args_string) {
  std::filesystem::path executable = find_executable(cmd);
  if (executable.empty()) {
    std::cout << cmd << ": command not found" << std::endl;
    return;
  }

  system((cmd + " " + args_string).c_str());
}

void handle_cmd(const std::string &cmd, const std::string &args_string) {
  if (all_cmds.contains(cmd)) {
    handle_shell_builtin(cmd, args_string);
  } else {
    handle_external_cmd(cmd, args_string);
  }
}

void handle_input(const std::string &input) {
  std::stringstream ss(input);
  std::string cmd;
  std::string args;
  ss >> cmd;
  std::getline(ss >> std::ws, args);

  handle_cmd(cmd, args);
}

void print_prompt() { std::cout << "$ "; }

void main_loop() {
  while (true) {
    print_prompt();
    std::string input;
    std::getline(std::cin, input);
    handle_input(input);
  }
}

void setup() {
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
}

int main() {
  setup();
  main_loop();
}
