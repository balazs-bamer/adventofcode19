#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>

int main(int const argc, char **argv) {
  try {
    if(argc == 1) {
      throw std::invalid_argument("Need input filename.");
    }
    std::ifstream in(argv[1]);
    std::unordered_map<std::string, std::string> orbits;
    while(true) {
      std::string line;
      std::getline(in, line);
      if(!in.good()) {
        break;
      }
      size_t separatorIndex = line.find(')');
      if(separatorIndex == std::string::npos) {
        throw std::invalid_argument("Invalid line in input.");
      }
      else {
        orbits[line.substr(separatorIndex + 1u, std::string::npos)] = line.substr(0, separatorIndex);
      }
    }
    std::string com{"COM"};
    int sum = 0;

    // There is an O(n) algorithm to do this, but I was too lazy to implement it.
    // Each value in the map should store the route length from that key to COM if known.
    // Initially all such stored values are unknown.
    // For each leaf, lookup should be performed until a known length is found, or to COM if none.
    // Travelling backwards (using an auxiliary list) gives all the length on this path.
    // Thus time is theta(2n). Space can be +theta(n) over the O(n^2) solution below.

    std::for_each(orbits.begin(), orbits.end(), [&sum, &orbits, &com](auto i){
      ++sum;
      auto j = orbits.find(i.first);
      while(j != orbits.end() && j->second != com) {
        j = orbits.find(j->second);
        ++sum;
      }
    });
    std::cout << sum << std::endl;
  }
  catch(std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
