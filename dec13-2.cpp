#include <infint.h>
#include <map>
#include <list>
#include <limits>
#include <deque>
#include <cctype>
#include <vector>
#include <string>
#include <fstream>
#include <utility>
#include <iostream>
#include <stdexcept>
#include <algorithm>

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

template<typename tNumber>
class Player final {
private:
  static int    constexpr cEmpty       =  0;
  static int    constexpr cWall        =  1;
  static int    constexpr cBlock       =  2;
  static int    constexpr cPaddle      =  3;
  static int    constexpr cBall        =  4;
  static int    constexpr cFree        =  2;
  static size_t constexpr cQuarterSlot =  0u;
  static int    constexpr cScore       = -1;
  static int    constexpr cLeft        = -1;
  static int    constexpr cRight       =  1;
  static int    constexpr cStay        =  0;

  Intcode<tNumber> mComputer;
  std::map<std::pair<int, int>, int> mScreen;
  int mPaddleMin, mPaddleMax; // don't know how big the paddle is

public:
  Player(std::ifstream &aIn) : mComputer(aIn) {
    mComputer.start();
    mComputer.poke(cQuarterSlot, tNumber(cFree));
    mComputer.run();
    draw();
    mPaddleMin = std::numeric_limits<int>::max();
    mPaddleMax = std::numeric_limits<int>::min();
    for(auto &i : mScreen) {
      if(i.second == cPaddle) {
        mPaddleMin = std::min<int>(mPaddleMin, i.first.first);
        mPaddleMax = std::max<int>(mPaddleMax, i.first.first);
      }
      else { // nothing to do
      }
    }
    if(mPaddleMin > mPaddleMax) {
      throw std::invalid_argument("No paddle on screen.");
    }
    else { // nothing to do
    }
  }

  void draw() {
    while(mComputer.hasOutput()) {
      std::pair<int, int> coordinates{mComputer.output().toInt(), mComputer.output().toInt()};
      int value = mComputer.output().toInt();
      if(coordinates.first == cScore) {
        std::cout << "Score: " << value << '\n';
      }
      else if(value == cEmpty) {
        auto found = mScreen.find(coordinates);
        if(found != mScreen.end()) {
          mScreen.erase(found);
        }
        else { // nothing to do
        }
      }
      else {
        mScreen[coordinates] = value;
      }
    }
  }
 
  void play() {
    while(!mComputer.run()) {
      draw();
      int ball = findBall();
      if(mPaddleMin > ball) {
        --mPaddleMin;
        --mPaddleMax;
        mComputer.input(cLeft);
      }  
      else if(mPaddleMax < ball) {
        ++mPaddleMin;
        ++mPaddleMax;
        mComputer.input(cRight);
      }  
      if(mPaddleMin > ball) {
        mComputer.input(cStay);
      }  
    }
    draw();
  }

private:
  int findBall() const {
    auto ball = std::find_if(mScreen.begin(), mScreen.end(), [](auto &i){
      return i.second == cBall;
    });
    if(ball == mScreen.end()) {
      throw std::invalid_argument("No ball on screen.");
    }
    else { // nothing to do
    }
    return ball->first.first;
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
    
    Player<Int> player(in);
    player.play();
  }
  catch(std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
