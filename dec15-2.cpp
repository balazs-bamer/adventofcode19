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

struct Coordinates final {
public:
  static int    constexpr cDeltaX[]    = { 0, 0, -1, 1};
  static int    constexpr cDeltaY[]    = {-1, 1,  0, 0};
  static int    constexpr cBackwards[] = { 1, 0,  3, 2};
  int x = 0;
  int y = 0;

  Coordinates() = default;
  
  Coordinates(int const aX, int const aY) noexcept : x(aX), y(aY) {
  }
  
  Coordinates operator+(int const aDirection) const noexcept {
    return Coordinates(x + cDeltaX[aDirection] , y + cDeltaY[aDirection]);
  }
  
  Coordinates& operator+=(int const aDirection) noexcept {
    x += cDeltaX[aDirection];
    y += cDeltaY[aDirection];
    return *this;
  }

  bool isOrigin() const noexcept {
    return x == 0 && y == 0;
  }

  bool operator==(Coordinates const &aOther) const noexcept {
    return x == aOther.x && y == aOther.y;
  }

  void decrease(Coordinates const &aOther) noexcept {
    x = std::min<int>(x, aOther.x);
    y = std::min<int>(y, aOther.y);
  }

  void increase(Coordinates const &aOther) noexcept {
    x = std::max<int>(x, aOther.x);
    y = std::max<int>(y, aOther.y);
  }
};

int constexpr Coordinates::cDeltaX[];
int constexpr Coordinates::cDeltaY[];
int constexpr Coordinates::cBackwards[];

template<>
struct std::hash<Coordinates> {
  size_t operator()(Coordinates const &aKey) const {
    return std::hash<int>{}(aKey.x) ^ (std::hash<int>{}(aKey.y) << 1u);
  }
};

class Node final {
public:
  static constexpr size_t cDirectionCount =  4u;
  static constexpr int    cNoParent       = -1;

private:
  // index is the <number to control the robot> - 1
  std::array<bool, cDirectionCount> mChildren;
  // Leave so for origin
  int   mParentDirection  = cNoParent;
  bool  mInitialized      = false;
  int   mDirectionCounter = 0;

public:
  // for origin
  Node() noexcept {
    std::fill(mChildren.begin(), mChildren.end(), false);
  }

  // for child 
  Node(int const aDirectionFromParent) noexcept : Node() {
    mParentDirection = Coordinates::cBackwards[aDirectionFromParent];
  }

  void makeChild(int const aDirectionToChild) noexcept {
    mChildren[aDirectionToChild] = true;
  }  
  
  bool isInitialized() const noexcept {
    return mInitialized;
  }

  bool setInitialized() noexcept {
    mInitialized = true;
  }

  int getParentDirection() const noexcept {
    return mParentDirection;
  }

  int getAndAdvanceDirectionCounter() noexcept {
    for(; mDirectionCounter < cDirectionCount && !mChildren[mDirectionCounter]; ++mDirectionCounter) {
    }
    int result = mDirectionCounter;
    mDirectionCounter = std::min<int>(mDirectionCounter + 1, cDirectionCount);
    return result;
  }

  void resetDirectionCounter() noexcept {
    mDirectionCounter = 0;
  }
};

class Labyrinth final {
private:
  static int    constexpr cOffset      =  1;
  static int    constexpr cWall        =  0;
  static int    constexpr cMoved       =  1;
  static int    constexpr cFree        =  1;
  static int    constexpr cOxygen      =  2;

  Intcode<Int>          mComputer;
  std::unordered_map<Coordinates, Node> mMap;
  bool                  mChanged    = false;
  Coordinates           mLocation;
  Coordinates           mOxygen;
  Coordinates           mUpperLeft;
  Coordinates           mLowerRight;

public:
  Labyrinth(std::ifstream &aIn)
  : mComputer(aIn)
  , mLocation(0, 0) 
  , mUpperLeft(std::numeric_limits<int>::max(), std::numeric_limits<int>::max())
  , mLowerRight(std::numeric_limits<int>::min(), std::numeric_limits<int>::min()) {
    mComputer.start();
  }

  void findOxygenAndExplore() {
    mMap.emplace(mLocation, Node());
    mUpperLeft.decrease(mLocation);
    mLowerRight.increase(mLocation);
    while(true) {
      Node &actual = mMap[mLocation];
      int newDirection;
      if(actual.isInitialized()) {
        newDirection = step(actual);
      }
      else {
        newDirection = initialize(actual);
      }
      if(newDirection == Node::cNoParent) {
        if(!mChanged) {
          break;
        }
        else { // nothing to do
        }
        mChanged = false;
      }
      else {
        mLocation += newDirection;
        mUpperLeft.decrease(mLocation);
        mLowerRight.increase(mLocation);
        if(moveRobot(newDirection) == cOxygen) {
          mOxygen = mLocation;
        }
        else { // nothing to do
        }
      }
    }
  }

  int fill() {
    int width = mLowerRight.x - mUpperLeft.x + 1u;
    int height = mLowerRight.y - mUpperLeft.y + 1u;
    std::vector<std::vector<int>> map(width);
    for(int i = 0; i < width; ++i) {
      map[i] = std::vector<int>(height, -2);
    }
    for(auto &i : mMap) {
      map[i.first.x - mUpperLeft.x][i.first.y - mUpperLeft.y] = -1;
    }
    map[mOxygen.x - mUpperLeft.x][mOxygen.y - mUpperLeft.y] = 0;
    size_t filled = 1u;
    int time = 0;
    while(filled < mMap.size()) {
      for(int x = 0; x < width; ++x) {
        for(int y = 0; y < height; ++y) {
          if(map[x][y] == time) {
            if(x > 0 && map[x - 1][y] == -1) {
              map[x - 1][y] = time + 1;
              ++filled;
            }
            if(x < width - 1 && map[x + 1][y] == -1) {
              map[x + 1][y] = time + 1;
              ++filled;
            }
            if(y > 0 && map[x][y - 1] == -1) {
              map[x][y - 1] = time + 1;
              ++filled;
            }
            if(y < height - 1 && map[x][y + 1] == -1) {
              map[x][y + 1] = time + 1;
              ++filled;
            }
          }
        }
      }
      ++time;
    }
    return time;
  }

private:
  int initialize(Node &aActual) noexcept {
    for(int i = 0; i < Node::cDirectionCount; ++i) {
      auto newLocation = mLocation + i;
      if(mMap.find(newLocation) == mMap.end()) { // not seen or wall
        int result = moveRobot(i);
        if(result == cMoved || result == cOxygen) {
          mMap.emplace(newLocation, i);
          aActual.makeChild(i);
          moveRobot(Coordinates::cBackwards[i]);
          mChanged = true;
        }
        else { // nothing to do
        }
      }
      else { // nothing to do
      }
    }
    aActual.setInitialized();
    return aActual.getParentDirection();
  }

  int step(Node &aActual) noexcept {
    int newDirection = aActual.getAndAdvanceDirectionCounter();
    if(newDirection == Node::cDirectionCount) {
      aActual.resetDirectionCounter();
      newDirection = aActual.getParentDirection();
    }
    else { // nothing to do
    }
    return newDirection;
  }

  int moveRobot(int const aDirection) {
    mComputer.input(aDirection + cOffset);
    mComputer.run();
    return mComputer.output().toInt();
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
    labyrinth.findOxygenAndExplore();
    int timeToFill = labyrinth.fill();
    auto end = std::chrono::high_resolution_clock::now();
    auto timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
    std::cout << "duration: " << timeSpan.count() << '\n';
    std::cout << timeToFill << '\n';
  }
  catch(std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
