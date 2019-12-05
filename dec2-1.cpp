#include <deque>
#include <cctype>
#include <fstream>
#include <iostream>
#include <stdexcept>

int main(int const argc, char **argv) {
  try {
    if(argc == 1) {
      throw std::invalid_argument("Need input filename.");
    }
    std::ifstream in(argv[1]);
    std::deque<int> memory;
    int sum = 0;
    while(true) {
      while(in.good() && !std::isdigit(in.peek())) {
        in.ignore(1u);
      }
      int integer;
      in >> integer;
      if(!in.good()) {
        break;
      }
      memory.push_back(integer);
    }

    memory[1u] = 12;
    memory[2u] = 2;

    size_t const cInstLength = 4u;
    int const cAdd = 1;
    int const cMultiply = 2;
    int const cHalt = 99;

    size_t programCounter = 0u;
    while(programCounter <= memory.size() - cInstLength) {
      int opcode = memory[programCounter];
      if(opcode == cAdd) {
        memory[memory[programCounter + 3u]] = memory[memory[programCounter + 1u]] + memory[memory[programCounter + 2u]];
      }
      else if(opcode == cMultiply) {
        memory[memory[programCounter + 3u]] = memory[memory[programCounter + 1u]] * memory[memory[programCounter + 2u]];
      }
      else if(opcode == cHalt) {
        break;
      }
      else {
        throw std::invalid_argument("Invalid opcode.");
      }
      programCounter += cInstLength;
    }
   
    std::cout << memory[0];
  }
  catch(std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
