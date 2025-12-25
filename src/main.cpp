#include "mai_app.h"
#include <cstdlib>
#include <exception>
#include <iostream>

int main() {
  MaiApp app;
  try {
    app.run();
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
