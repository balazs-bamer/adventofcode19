#include <deque>
#include <cctype>
#include <fstream>
#include <iostream>
#include <stdexcept>


size_t getAddress(std::deque<int> const &aMemory, size_t const aProgramCounter, int const aOffset) noexcept {
  int const dividor[] = {0, 100, 1000, 10000};
  int digit = (aMemory[aProgramCounter] / dividor[aOffset]) % 10;
  size_t result;
  if(digit == 0) {
    result = aMemory[aProgramCounter + aOffset];
  }
  else {
    result = aProgramCounter + aOffset;
  }
  return result;
}

int main(int const argc, char **argv) {
  try {
    if(argc == 1) {
      throw std::invalid_argument("Need input filename.");
    }
    std::ifstream in(argv[1]);
    std::deque<int> program;
    while(true) {
      int integer;
      in >> integer;
      if(!in.good()) {
        break;
      }
      program.push_back(integer);
      in.ignore(1u);
    }

    size_t const  cInstLengths[] = {1u, 4u, 4u, 2u, 2u, 3u, 3u, 4u, 4u};
    int const cAdd        =  1;
    int const cMultiply   =  2;
    int const cInput      =  3;
    int const cOutput     =  4;
    int const cJumpIfNot0 =  5;
    int const cJumpIf0    =  6;
    int const cLessThan   =  7;
    int const cEquals     =  8;
    int const cHalt       = 99;
    int const cMaskOpcode = 100;
    size_t  const cOffsetParameter1  =     1u;
    size_t  const cOffsetParameter2  =     2u;
    size_t  const cOffsetResult      =     3u;

    std::deque<int> memory(program);

    size_t programCounter = 0u;
    while(programCounter <= memory.size()) {
      int opcode = memory[programCounter] % cMaskOpcode;
    
      bool jumped = false; 
      if(opcode == cAdd) {
        size_t addressParameter1 = getAddress(memory, programCounter, cOffsetParameter1);
        size_t addressParameter2 = getAddress(memory, programCounter, cOffsetParameter2);
        size_t addressResult     = getAddress(memory, programCounter, cOffsetResult);
        memory[addressResult] = memory[addressParameter1] + memory[addressParameter2];
      }
      else if(opcode == cMultiply) {
        size_t addressParameter1 = getAddress(memory, programCounter, cOffsetParameter1);
        size_t addressParameter2 = getAddress(memory, programCounter, cOffsetParameter2);
        size_t addressResult     = getAddress(memory, programCounter, cOffsetResult);
        memory[addressResult] = memory[addressParameter1] * memory[addressParameter2];
      }
      else if(opcode == cInput) {
        size_t addressParameter1 = getAddress(memory, programCounter, cOffsetParameter1);
        memory[addressParameter1] = 5;
      }
      else if(opcode == cOutput) {
        size_t addressParameter1 = getAddress(memory, programCounter, cOffsetParameter1);
        std::cout << memory[addressParameter1] << '\n';
      }
      else if(opcode == cJumpIfNot0) {
        size_t addressToCheck = getAddress(memory, programCounter, cOffsetParameter1);
        size_t addressOfJUmp = getAddress(memory, programCounter, cOffsetParameter2);
        if(memory[addressToCheck] != 0) {
          programCounter = memory[addressOfJUmp];
          jumped = true;
        }
        else { // nothing to do
        }
      }
      else if(opcode == cJumpIf0) {
        size_t addressToCheck = getAddress(memory, programCounter, cOffsetParameter1);
        size_t addressOfJUmp = getAddress(memory, programCounter, cOffsetParameter2);
        if(memory[addressToCheck] == 0) {
          programCounter = memory[addressOfJUmp];
          jumped = true;
        }
        else { // nothing to do
        }
      }
      else if(opcode == cLessThan) {
        size_t addressParameter1 = getAddress(memory, programCounter, cOffsetParameter1);
        size_t addressParameter2 = getAddress(memory, programCounter, cOffsetParameter2);
        size_t addressResult     = getAddress(memory, programCounter, cOffsetResult);
        memory[addressResult] = (memory[addressParameter1] < memory[addressParameter2] ? 1 : 0);
      }
      else if(opcode == cEquals) {
        size_t addressParameter1 = getAddress(memory, programCounter, cOffsetParameter1);
        size_t addressParameter2 = getAddress(memory, programCounter, cOffsetParameter2);
        size_t addressResult     = getAddress(memory, programCounter, cOffsetResult);
        memory[addressResult] = (memory[addressParameter1] == memory[addressParameter2] ? 1 : 0);
      }
      else if(opcode == cHalt) {
        break;
      }
      else {
        throw std::invalid_argument("Invalid opcode.");
      }
      if(!jumped) {
        programCounter += cInstLengths[opcode];
      }
      else { // nothing to do
      }
    }
  }
  catch(std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
