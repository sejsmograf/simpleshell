#include <iostream>

void setup() {
  std::cout << std::unitbuf; // Enable automatic fluhsing after any output
  std::cerr << std::unitbuf; // for both std and err
}

int main() {
  setup();
  return 0;
}
