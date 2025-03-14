#include <iostream>
#include <string>

void handle_input(const std::string &input) {
  std::cout << "Command not found";
}

void print_prompt() { std::cout << "$ "; }

void main_loop() {
  print_prompt();
  std::string input;
  std::getline(std::cin, input);
  handle_input(input);
}

void setup() {
  std::cout << std::unitbuf; // Enable automatic fluhsing after any output
  std::cerr << std::unitbuf; // for both std and err
}

int main() {
  setup();
  return 0;
}
