#include <set>
#include <list>
#include <array>
#include <deque>
#include <limits>
#include <chrono>
#include <cctype>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <utility>
#include <optional>
#include <iostream>
#include <functional>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>

// Both parts use hardcoded scripts.
// Script parsing, interpreting and output rendering implemented in interpreted Intcode
// lasts way too long to use under a heuristic search algorithm.
// I don't feel like disassembling and hacking Intcode to make it feasible.

class Int final {
private:
  int mInt;

public:
  Int(int aInt) noexcept : mInt(aInt) {
  }

  Int(std::string &aString) : mInt(std::stoi(aString)) {
  }

  int toInt() const noexcept {
    return mInt;
  }

  operator int() const noexcept {
    return mInt;
  }
};

template<typename tNumber>
class Intcode final {
private:
  static size_t constexpr cInstLengths[] = {1u, 4u, 4u, 2u, 2u, 3u, 3u, 4u, 4u, 2u, 1u};
  static int    const cAdd              =  1;
  static int    const cMultiply         =  2;
  static int    const cInput            =  3;
  static int    const cOutput           =  4;
  static int    const cJumpIfNot0       =  5;
  static int    const cJumpIf0          =  6;
  static int    const cLessThan         =  7;
  static int    const cEquals           =  8;
  static int    const cRelativeBase     =  9;
  static int    const cInstCount        = 10;
  static int    const cHalt             = 99;
  static int    const cMaskOpcode       = 100;
  static size_t const cOffsetParameter1 =   1u;
  static size_t const cOffsetParameter2 =   2u;
  static size_t const cOffsetResult     =   3u;

  std::list<tNumber>  mInputs;
  std::list<tNumber>  mOutputs;
  std::deque<tNumber> mProgram;
  std::deque<tNumber> mMemory;
  size_t              mProgramCounter;
  size_t              mRelativeBase;

public:
  Intcode() noexcept = default;

  Intcode(std::ifstream &aIn) noexcept {
    while(true) {
      std::string number;
      std::getline(aIn, number, ',');
      if(!aIn.good()) {
        break;
      }
      tNumber integer(number);
      mProgram.push_back(integer);
    }
  }

  Intcode(Intcode const &aOther) noexcept : mProgram(aOther.mProgram) {
  }

  void input(int const aInput) noexcept {
    mInputs.push_back(aInput);
  }

  void input(std::string const &aInput) noexcept {
    for(auto &i : aInput) {
      mInputs.push_back(static_cast<int>(i));
    }
  }

  tNumber output() {
    return get(mOutputs);
  }
  
  bool hasOutput() {
    return !mOutputs.empty();
  }

  void start() {
    mInputs.clear();
    mOutputs.clear();
    mMemory = mProgram;
    mProgramCounter = 0u;
    mRelativeBase   = 0u;
  }

  void poke(size_t const aLocation, tNumber const &aValue) {
    expand(aLocation);
    mMemory[aLocation] = aValue;
  }

  bool run() {
    bool result;
    while(true) {
      if(mProgramCounter >= mMemory.size()) {
        throw std::invalid_argument("Invalid program.");
      }
      else { // nothing to do
      }
      tNumber opcode = mMemory[mProgramCounter] % cMaskOpcode;
      if(opcode != cHalt && opcode >= cInstCount) {
        throw std::invalid_argument("Invalid program.");
      }
      else { // nothing to do
      }
    
      bool jumped = false; 
      if(opcode == cAdd) {
        size_t addressParameter1 = getAddress(cOffsetParameter1);
        size_t addressParameter2 = getAddress(cOffsetParameter2);
        size_t addressResult     = getAddress(cOffsetResult);
        mMemory[addressResult] = mMemory[addressParameter1] + mMemory[addressParameter2];
      }
      else if(opcode == cMultiply) {
        size_t addressParameter1 = getAddress(cOffsetParameter1);
        size_t addressParameter2 = getAddress(cOffsetParameter2);
        size_t addressResult     = getAddress(cOffsetResult);
        mMemory[addressResult] = mMemory[addressParameter1] * mMemory[addressParameter2];
      }
      else if(opcode == cInput) {
        size_t addressParameter1 = getAddress(cOffsetParameter1);
        if(mInputs.size() == 0u) {
          result = false;
          break;
        }
        else {
          mMemory[addressParameter1] = get(mInputs);
        }
      }
      else if(opcode == cOutput) {
        size_t addressParameter1 = getAddress(cOffsetParameter1);
        mOutputs.push_back(mMemory[addressParameter1]);
      }
      else if(opcode == cJumpIfNot0) {
        size_t addressToCheck = getAddress(cOffsetParameter1);
        size_t addressOfJUmp = getAddress(cOffsetParameter2);
        if(mMemory[addressToCheck] != 0) {
          mProgramCounter = mMemory[addressOfJUmp].toInt();
          jumped = true;
        }
        else { // nothing to do
        }
      }
      else if(opcode == cJumpIf0) {
        size_t addressToCheck = getAddress(cOffsetParameter1);
        size_t addressOfJUmp = getAddress(cOffsetParameter2);
        if(mMemory[addressToCheck] == 0) {
          mProgramCounter = mMemory[addressOfJUmp].toInt();
          jumped = true;
        }
        else { // nothing to do
        }
      }
      else if(opcode == cLessThan) {
        size_t addressParameter1 = getAddress(cOffsetParameter1);
        size_t addressParameter2 = getAddress(cOffsetParameter2);
        size_t addressResult     = getAddress(cOffsetResult);
        mMemory[addressResult] = (mMemory[addressParameter1] < mMemory[addressParameter2] ? 1 : 0);
      }
      else if(opcode == cEquals) {
        size_t addressParameter1 = getAddress(cOffsetParameter1);
        size_t addressParameter2 = getAddress(cOffsetParameter2);
        size_t addressResult     = getAddress(cOffsetResult);
        mMemory[addressResult] = (mMemory[addressParameter1] == mMemory[addressParameter2] ? 1 : 0);
      }
      else if(opcode == cRelativeBase) {
        size_t addressParameter1 = getAddress(cOffsetParameter1);
        mRelativeBase += mMemory[addressParameter1].toInt();
      }
      else if(opcode == cHalt) {
        result = true;
        break;
      }
      else { // nothing to do
      }
      if(!jumped) {
        mProgramCounter += cInstLengths[opcode.toInt()];
      }
      else { // nothing to do
      }
    }
    return result;
  }

private:
  size_t getAddress(size_t const aOffset) {
    tNumber const dividor[] = {0, 100, 1000, 10000};
    tNumber digit = (mMemory[mProgramCounter] / dividor[aOffset]) % 10;
    size_t result;
    if(digit == 0) {
      result = mMemory[mProgramCounter + aOffset].toInt();
    }
    else if(digit == 2) {
      result = mMemory[mProgramCounter + aOffset].toInt() + mRelativeBase;
    }
    else { // else 1, immediate
      result = mProgramCounter + aOffset;
    }
    expand(result);
    return result;
  }

  void expand(size_t const aLocation) {
    if(aLocation >= mMemory.size()) {
      tNumber zero{0};
      mMemory.resize(aLocation + 1u, zero);
    }
    else { // nothing to do
    }
  }
  
  tNumber get(std::list<tNumber> &aList) {
    if(aList.size() == 0u) {
      throw std::invalid_argument("List empty.");
    }
    else { // nothing to do
    }
    tNumber result = aList.front();
    aList.pop_front();
    return result;
  }
};

template<typename tNumber>
size_t constexpr Intcode<tNumber>::cInstLengths[];

enum class Opcode : uint8_t {
  cIllegal = 0u,
  cNot     = 1u,
  cOr      = 2u,
  cAnd     = 3u,
  cWalk    = 4u,
  cRun     = 5u
};

enum class Operand : uint8_t {
  cA           =  0u,
  cB           =  1u,
  cC           =  2u,
  cD           =  3u,
  cE           =  4u,
  cF           =  5u,
  cG           =  6u,
  cH           =  7u,
  cI           =  8u,
  cSensorCount =  9u,
  cT           =  9u,
  cJ           = 10u
};

class Instruction final {
  friend std::ostream& operator<<(std::ostream &os, const Instruction&);
private:
  static constexpr size_t cOpcodeLength = 5u;
  static constexpr size_t cOperandLength = 5u;
  static constexpr char   cOpcodes[][cOpcodeLength] = { "ILL", "NOT", "OR", "AND", "WALK", "RUN" };
  static constexpr char   cOperands[][cOperandLength] = { "A", "B", "C", "D", "E", "F", "G", "H", "I", "T", "J" };

  Opcode  mOpcode = Opcode::cWalk;
  Operand mSource;
  Operand mDest;

public:
  Instruction() noexcept = default;

  Instruction(bool const aRun) noexcept : mOpcode(aRun ? Opcode::cRun : Opcode::cWalk) {
  }  

  Instruction(Opcode const aOpcode, Operand const aSrc, Operand const aDest) noexcept : mOpcode(aOpcode), mSource(aSrc), mDest(aDest) {
  }

  Instruction &operator=(Instruction const &aOther) noexcept = default;
};

std::ostream& operator<<(std::ostream &aOut, Instruction const &aWhat) {
  aOut << aWhat.cOpcodes[static_cast<size_t>(aWhat.mOpcode)];
  if(aWhat.mOpcode != Opcode::cWalk && aWhat.mOpcode != Opcode::cRun) {
    aOut << ' ' << aWhat.cOperands[static_cast<size_t>(aWhat.mSource)] << ' ' << aWhat.cOperands[static_cast<size_t>(aWhat.mDest)];
  }
  else { // nothing to do
  }
  aOut << '\n';
  return aOut;
}
  
constexpr char Instruction::cOpcodes[][Instruction::cOpcodeLength];
constexpr char Instruction::cOperands[][Instruction::cOperandLength];

class Spring final {
private:
  static constexpr int cResultLimit = 127;

  Intcode<Int> mComputer;

public:
  Spring(std::ifstream &aIn) : mComputer(aIn) {
  }

  size_t compute() {
    size_t result = 0u;
    std::ostringstream out;
    out << Instruction(Opcode::cNot, Operand::cB, Operand::cJ);
    out << Instruction(Opcode::cNot, Operand::cC, Operand::cT);
    out << Instruction(Opcode::cOr,  Operand::cT, Operand::cJ);
    out << Instruction(Opcode::cAnd, Operand::cD, Operand::cJ);
    out << Instruction(Opcode::cAnd, Operand::cH, Operand::cJ);
    out << Instruction(Opcode::cNot, Operand::cA, Operand::cT);
    out << Instruction(Opcode::cOr,  Operand::cT, Operand::cJ);
    out << Instruction(true);
    mComputer.start();
    mComputer.run();
    while(mComputer.hasOutput()) {
      std::cout << static_cast<char>(mComputer.output().toInt());
    }
    mComputer.input(out.str());
    mComputer.run();
    while(mComputer.hasOutput()) {
      int number = mComputer.output().toInt();
      if(number > cResultLimit) {
        result = number;
      }
      else {
        std::cout << static_cast<char>(number);
      }
    }
    return result;
  }

private:
};

int main(int const argc, char **argv) {
  size_t const cChainLength = 5u;
  int const cInitialInput = 0;

  try {
    if(argc == 1) {
      throw std::invalid_argument("Need input filename.");
    }
    std::ifstream in(argv[1]);
    
    Spring spring(in);
    auto begin = std::chrono::high_resolution_clock::now();
    size_t result = spring.compute();
    auto end = std::chrono::high_resolution_clock::now();
    auto timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
    std::cout << "duration: " << timeSpan.count() << '\n';
    std::cout << result << '\n';
  }
  catch(std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
