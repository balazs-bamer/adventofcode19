#include <fstream>
#include <iostream>
#include <stdexcept>

int getTotalFuel(int const aMass) {
  int total = 0;
  int remaining = aMass;
  for(;;) {
    remaining = remaining / 3 - 2;
    if(remaining <= 0) {
      break;
    }
    total += remaining;
  }
  return total;
}

int main(int const aArgc, char **aArgv) {
  try {
    if(aArgc == 1) {
      throw std::invalid_argument("Need input filename.");
    }
    std::ifstream in(aArgv[1]);
    int sum = 0;
    while(true) {
      int weight;
      in >> weight;
      if(!in.good()) {
        break;
      }
      sum += getTotalFuel(weight);
    }
    std::cout << sum << std::endl;
  }
  catch(std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
