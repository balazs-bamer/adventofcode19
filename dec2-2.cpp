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
    std::deque<int> program;
    while(true) {
      while(in.good() && !std::isdigit(in.peek())) {
        in.ignore(1u);
      }
      int integer;
      in >> integer;
      if(!in.good()) {
        break;
      }
      program.push_back(integer);
    }

    size_t const  cInstLength = 4u;
    int const cAdd = 1;
    int const cMultiply = 2;
    int const cHalt = 99;
    size_t const  cAddressNoun = 1u;
    size_t const  cAddressVerb = 2u;

    int const cRangeStart = 0;
    int const cRangeEnd = 99 + 1;
    int const cTarget = 19690720;

    int noun, verb;
    for(noun = cRangeStart; noun < cRangeEnd; ++noun) {
      for(verb = cRangeStart; verb < cRangeEnd; ++verb) {
        std::deque<int> memory(program);
        memory[cAddressNoun] = noun;
        memory[cAddressVerb] = verb;

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
        if(memory[0u] == cTarget) {
          std::cout << 100 * noun + verb << '\n';
        }
      }
    }
  }
  catch(std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
