#include <set>
#include <list>
#include <array>
#include <deque>
#include <limits>
#include <chrono>
#include <cctype>
#include <vector>
#include <fstream>
#include <utility>
#include <optional>
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

struct PathPart final {
  int turn;
  int length;

  bool operator==(PathPart const &aOther) const noexcept {
    return turn == aOther.turn && length == aOther.length;
  }

  bool operator!=(PathPart const &aOther) const noexcept {
    return !(*this == aOther);
  }
};

size_t intLength(int const aNumber) noexcept {
  size_t const cMaxLength = 9u;
  int threshold = 10;
  size_t length = 1;
  while(length <= cMaxLength) {
    if(aNumber < threshold) {
      break;
    }
    else {
      threshold *= 10;
      ++length;
    }
  }
  return length;
}

class Path final {
public:
  static constexpr size_t cMaxStringLength = 20u;

private:
  std::deque<PathPart> mPath;

public:
  Path& operator+=(PathPart const &aPart) {
    mPath.push_back(aPart);
std::cout << aPart.turn << ' ' << aPart.length << '\n';
    return *this;
  }

  PathPart get(size_t const aAt) const {
    return mPath[aAt];
  }

  bool aligns(Path const &aOther, size_t const aOffset) const {
    bool aligns = true;
    for(size_t i = 0; i < aOther.size(); ++i) {
      if(aOther.mPath[i] != mPath[i + aOffset]) {
        aligns = false;
        break;
      }
      else { // nothing to do
      }
    }
    return aligns;
  }

  bool aligns(Path const &aOther, size_t const aOffset, std::deque<bool> const &aOccupied) const {
    bool aligns = true;
    for(size_t i = 0; i < aOther.size(); ++i) {
      if(aOther.mPath[i] != mPath[i + aOffset] || aOccupied[i + aOffset]) {
        aligns = false;
        break;
      }
      else { // nothing to do
      }
    }
    return aligns;
  }

  bool check() const noexcept {
    size_t result = 0u;
    if(!mPath.empty()) {
      result = mPath.size() * 3u - 1u;
      for(auto &i : mPath) {
        result += intLength(i.length);
      }
    }
    else { // nothong to do
    }
    return result <= cMaxStringLength;
  }

  size_t size() const noexcept {
    return mPath.size();
  }
};

struct Reference final {
  size_t start;
  size_t reference;

  bool operator<(Reference const aOther) const noexcept {
    return start < aOther.start;
  }
};


class Experiment final {
public:
  static constexpr size_t cSubCount   = 3u; 

private:
  Path const                 *mMaster;
  std::deque<bool>            mOccupied;
  std::set<Reference>         mReferences;
  std::array<Path, cSubCount> mSubs;
  bool                        mBroken = true;

public:
  Experiment(Path const *aMaster) : mMaster(aMaster), mOccupied(std::deque<bool>(mMaster->size(), false)) {
  }

  Experiment(Experiment const &aOther, size_t const aIndex) 
  : mMaster(aOther.mMaster)
  , mOccupied(aOther.mOccupied)
  , mReferences(aOther.mReferences)
  , mSubs(aOther.mSubs)
  , mBroken(false) {
    size_t found = 0;
    for(; found < mOccupied.size(); ++found) {
      if(!mOccupied[found]) {
        break;
      }
    }
    mSubs[aIndex] += mMaster->get(found);
std::cout << "sub: " << aIndex << ' ' << mMaster->get(found).turn << ' ' << mMaster->get(found).length << '\n';
  }

  Experiment(Experiment const &aOther, size_t const aIndex, size_t const aAt) 
  : mMaster(aOther.mMaster)
  , mOccupied(aOther.mOccupied)
  , mReferences(aOther.mReferences)
  , mSubs(aOther.mSubs)
  , mBroken(false) {
    if(aIndex >= cSubCount) {
      Reference ref;
      ref.start = aAt;
      ref.reference = aIndex - cSubCount;
std::cout << "ref: " << ref.reference<< ' ' << aAt << '\n';
      mReferences.insert(ref);
      for(size_t i = 0u; i < mSubs[ref.reference].size(); ++i) {
        mOccupied[i + aAt] = true;
      } 
    }
    else {
      PathPart part = mMaster->get(aAt);
      for(auto &i : mReferences) {
        size_t pos = i.start + mSubs[aIndex].size();
        if(pos >= mMaster->size() || mOccupied[pos] || mMaster->get(pos) != part) {
          mBroken = true;
          break;
        }
      }
      mSubs[aIndex] += part;
    }
  }

  Experiment(Experiment&&) = default;
  Experiment(Experiment const&) = default;

  Experiment &operator=(Experiment const&) = default;
  Experiment &operator=(Experiment &&) = default;

  bool isBroken() const noexcept {
    return mBroken;
  }

  bool check() const {
    bool success;
    if(mReferences.empty()) {
      success = true;
    }
    else {
      bool success = !mBroken && (mReferences.size() * 2u - 1u <= Path::cMaxStringLength);
      for(auto &sub : mSubs) {
        if(!sub.check()) {
          success = false;
        }
        else { // nothing  to do
        }
      }
      for(auto &ref : mReferences) {
        if(!mMaster->aligns(mSubs[ref.reference], ref.start)) {
          success = false;
        }
        else { // nothing  to do
        }
      }
    }
    return success;
  }

  // Does not checks correctness, it is extra task with check()
  // called before pushing into the queue
  bool isReady() const {
    size_t covered = 0u;
    for(auto &ref : mReferences) {
      covered += mSubs[ref.reference].size();
    }
    return covered = mMaster->size();
  }

  std::optional<Experiment> addChildren(std::list<Experiment> &aList) {
    std::optional<Experiment> result;
    for(size_t i = 0u; i < cSubCount; ++i) {
      if(mSubs[i].size() == 0u) {
        Experiment newExperiment(*this, i);
        if(newExperiment.check()) {
          aList.push_back(newExperiment);
        }
        else { // nothing to do
        }
      }
      else {
        for(auto &ref : mReferences) {
          size_t pos = ref.start + mSubs[i].size();
          if(ref.reference == i && pos < mMaster->size() && !mOccupied[pos]) {
            Experiment newExperiment(*this, i, pos);
            if(newExperiment.check()) {
              aList.push_back(newExperiment);
            }
            else { // nothing to do
            }
          }
        }
      }
    }
    for(size_t i = 0u;  i < cSubCount && mSubs[i].size() > 0u; ++i) {
      for(size_t found = 0u; found <= mMaster->size() - mSubs[i].size(); ++found) {
        if(mMaster->aligns(mSubs[i], found, mOccupied)) {
          Experiment newExperiment(*this, i + cSubCount, found);
          if(newExperiment.check()) {
            if(newExperiment.isReady()) {
              result = std::move(newExperiment);
              return result;
              break;
            }
            else {
              aList.push_back(newExperiment);
            }
          }
          else { // nothing to do
          }
        }
      } 
    }
    return result;
  }
};

class Labyrinth final {
private:
  static int constexpr cScaffold  = '#';
  static int constexpr cSpace     = '.';
  static int constexpr cNewline   = 10;
  static int constexpr cCharUp    = '^'; 
  static int constexpr cCharRight = '>';
  static int constexpr cCharDown  = 'v';
  static int constexpr cCharLeft  = '<';
  static int constexpr cUp        = 0; 
  static int constexpr cRight     = 1;
  static int constexpr cDown      = 2;
  static int constexpr cLeft      = 3;
  static int constexpr cDeltaX[]  = {0, 1, 0, -1}; 
  static int constexpr cDeltaY[]  = {-1, 0, 1, 0}; 
  static int constexpr cDirCount  = 4;

  Intcode<Int>                mComputer;
  std::deque<std::deque<int>> mMap;
  Path                        mPath;
  int                         mWidth  = 0u;
  int                         mHeight = 0u;
  int                         mRobotStartX;
  int                         mRobotStartY;
  int                         mRobotStartDir;

public:
  Labyrinth(std::ifstream &aIn)
  : mComputer(aIn) {
    mComputer.start();
  }
  
  // Appends a layer of space all around to make later life easier.
  void readScaffold() {
    mComputer.run();
    bool newline = true;
    int x = 0;
    int y = 1;
std::cout << "....";
    while(mComputer.hasOutput()) {
      if(newline) {
        mMap.push_back(std::deque<int>());
        mMap.back().push_back(cSpace);
std::cout << "\n.";
        x = 1;
        ++mHeight;
        newline = false;
      }
      else { // nothing to do
      }
      int input = mComputer.output();
      if(input == cNewline) {
        mMap.back().push_back(cSpace);
std::cout << ".";
        ++y;
        newline = true;
      }
      else {
std::cout << static_cast<char>(input);
        if(input == cCharUp) {
          mRobotStartDir = cUp;
          input = cScaffold;
          mRobotStartX = x;
          mRobotStartY = y;
        }
        else if(input == cCharRight) {
          mRobotStartDir = cRight;
          input = cScaffold;
          mRobotStartX = x;
          mRobotStartY = y;
        }
        else
        if(input == cCharDown) {
          mRobotStartDir = cDown;
          input = cScaffold;
          mRobotStartX = x;
          mRobotStartY = y;
        }
        else
        if(input == cCharLeft) {
          mRobotStartDir = cLeft;
          input = cScaffold;
          mRobotStartX = x;
          mRobotStartY = y;
        }
        else { // nothing to do
        }
        mMap.back().push_back(input);
        ++x;
        mWidth = std::max<int>(mWidth, mMap.back().size());
      }
    }
    if(mMap.back().empty()) {
      mMap.pop_back();
      --mHeight;
    }
    else {
      mMap.back().push_back(cSpace);
    }
    mHeight += 2u;
    mMap.push_front(std::deque<int>(mWidth, cSpace));
    mMap.push_back(std::deque<int>(mWidth, cSpace));

size_t sum = 0;
/*    for(size_t y = 0u; y < mHeight; ++y) {
      for(size_t x = 0u; x < mWidth; ++x) {
        if(mMap[y][x] == cScaffold) {
          int neighbours = (mMap[y][x - 1u] == cScaffold ? 1 : 0);
          neighbours += (mMap[y][x + 1] == cScaffold ? 1 : 0);
          neighbours += (mMap[y - 1u][x] == cScaffold ? 1 : 0);
          neighbours += (mMap[y + 1][x] == cScaffold ? 1 : 0);
          if(neighbours == 4) {
            sum += (x-1) * (y-1);
          }
          else { // nothing to do
          }
        }
        else { // nothing to do
        }
      }
    }*/
std::cout << "\n" << sum << ' ' << mRobotStartX << ' ' << mRobotStartY << '\n';;
  }

  void surveyPath() {
    int x = mRobotStartX;
    int y = mRobotStartY;
    int dir = mRobotStartDir;
    int newDir = getNewDir(x, y, cDirCount);
    int length = 0;
    PathPart pathPart;
    pathPart.turn = getTurn(dir, newDir);
    do {
      dir = newDir;
      x += cDeltaX[dir];
      y += cDeltaY[dir];
      ++length;
      newDir = getNewDir(x, y, dir);
      if(newDir == cDirCount) {
        pathPart.length = length;
        mPath += pathPart;
        break;
      }
      else if(newDir != dir) {
        pathPart.length = length;
        mPath += pathPart;
        pathPart.turn = getTurn(dir, newDir);
        length = 0;
      }
      else { // nothing to do
      }
    }while(true);
  }

  size_t collectDust() const {
    size_t sum = 0;
    std::list<Experiment> list;
    list.emplace_back(&mPath);
    std::optional<Experiment> found;
    while(!found && !list.empty()) {
      Experiment experiment = list.front();
      list.pop_front();
      found = experiment.addChildren(list);
std::cout << "l: " << list.size() << '\n';
    }
    if(found) {
std::cout << "f\n";
    }
    return sum;
  }

private:
  
  int getOppositeDir(int const aDir) const noexcept {
    return (aDir + 2) % cDirCount;
  }

  int getNewDir(int const aX, int const aY, int const aOldDir) const {
    int newDir;
    int found = 0;
    for(int i = 0; i < cDirCount; ++i) {
      if(i != getOppositeDir(aOldDir) && mMap[aY + cDeltaY[i]][aX + cDeltaX[i]] == cScaffold) {
        newDir = i;
        ++found;
      }
      else { // nothing to do
      }
    }
    if(found > 1) {
      newDir = aOldDir;
    }
    else if(found == 0) {
      newDir = cDirCount;
    }
    else { // nothing to do
    }
    return newDir;
  }

  int getTurn(int const aOldDir, int const aNewDir) const noexcept {
    return (cDirCount + aNewDir - aOldDir) % cDirCount;
  }
};

int constexpr Labyrinth::cSpace;
int constexpr Labyrinth::cDeltaX[];
int constexpr Labyrinth::cDeltaY[];

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
    labyrinth.surveyPath();
    size_t dust = labyrinth.collectDust();
    auto end = std::chrono::high_resolution_clock::now();
    auto timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
    std::cout << "duration: " << timeSpan.count() << '\n';
    std::cout << dust << '\n';
  }
  catch(std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
