#include <list>
#include <array>
#include <deque>
#include <limits>
#include <chrono>
#include <cctype>
#include <vector>
#include <fstream>
#include <utility>
#include <iostream>
#include <functional>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>

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

class Labyrinth final {
private:
  static int    constexpr cScaffold    = '#';
  static int    constexpr cSpace       = '.';
  static int    constexpr cNewline     = 10;

  Intcode<Int>                mComputer;
  std::deque<std::deque<int>> mMap;
  size_t                      mWidth  = 0u;
  size_t                      mHeight = 0u;

public:
  Labyrinth(std::ifstream &aIn)
  : mComputer(aIn) {
    mComputer.start();
  }
  
  void readScaffold() {
    mComputer.run();
    bool newline = true;
    while(mComputer.hasOutput()) {
      if(newline) {
        mMap.push_back(std::deque<int>());
        ++mHeight;
        newline = false;
      }
      else { // nothing to do
      }
      int input = mComputer.output();
      if(input == cNewline) {
        newline = true;
      }
      else {
        mMap.back().push_back(input);
        mWidth = std::max<size_t>(mWidth, mMap.back().size());
      }
    }
    if(mMap.back().empty()) {
      mMap.pop_back();
      --mHeight;
    }
    else { // nothing to do
    }
  }

  size_t calibrate() const {
    size_t sum = 0;
    for(size_t y = 0u; y < mHeight; ++y) {
      for(size_t x = 0u; x < mWidth; ++x) {
        if(mMap[y][x] == cScaffold) {
          int neighbours = (x > 0u && mMap[y][x - 1u] == cScaffold ? 1 : 0);
          neighbours += (x < mWidth - 1 && mMap[y][x + 1] == cScaffold ? 1 : 0);
          neighbours += (y > 0u && mMap[y - 1u][x] == cScaffold ? 1 : 0);
          neighbours += (y < mHeight - 1 && mMap[y + 1][x] == cScaffold ? 1 : 0);
          if(neighbours == 4) {
            sum += x * y;
          }
          else { // nothing to do
          }
        }
        else { // nothing to do
        }
      }
    }
    return sum;
  }
};

int main(int const argc, char **argv) {
  size_t const cChainLength = 5u;
  int const cInitialInput = 0;

  try {
    if(argc == 1) {
      throw std::invalid_argument("Need input filename.");
    }
    std::ifstream in(argv[1]);
    
    Labyrinth labyrinth(in);
    auto begin = std::chrono::high_resolution_clock::now();
    labyrinth.readScaffold();
    size_t calibration = labyrinth.calibrate();
    auto end = std::chrono::high_resolution_clock::now();
    auto timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
    std::cout << "duration: " << timeSpan.count() << '\n';
    std::cout << calibration << '\n';
  }
  catch(std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
