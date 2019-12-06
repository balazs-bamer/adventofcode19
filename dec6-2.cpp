#include <list>
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

    std::string const com{"COM"};
    std::string const me{"YOU"};
    std::string const santa{"SAN"};
    std::list<std::string> pathSanta;
    std::list<std::string> pathI;
    
    for(auto i = orbits.find(me); i != orbits.end(); i = orbits.find(i->second)) {
      pathI.push_front(i->second);
    }
    for(auto i = orbits.find(santa); i != orbits.end(); i = orbits.find(i->second)) {
      pathSanta.push_front(i->second);
    }
  
    int length = pathI.size() + pathSanta.size();
    for(auto i = pathI.begin(), j = pathSanta.begin(); i != pathI.end() && j != pathSanta.end() && *i == *j; ++i, ++j) {
      length -= 2;
    }

    std::cout << length << std::endl;
  }
  catch(std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
