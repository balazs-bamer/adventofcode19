#include <fstream>
#include <iostream>
#include <stdexcept>

int main(int const argc, char **argv) {
  try {
    if(argc == 1) {
      throw std::invalid_argument("Need input filename.");
    }
    std::ifstream in(argv[1]);
    int sum = 0;
    while(true) {
      int weight;
      in >> weight;
      if(!in.good()) {
        break;
      }
      int fuel = weight / 3 - 2;
      if(fuel > 0) {
        sum += fuel;
      }
    }
    std::cout << sum << std::endl;
  }
  catch(std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
