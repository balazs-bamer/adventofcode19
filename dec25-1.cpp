#include "infint.h"
#include <list>
#include <array>
#include <deque>
#include <limits>
#include <chrono>
#include <cctype>
#include <string>
#include <fstream>
#include <utility>
#include <optional>
#include <iostream>
#include <functional>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

// I don't suppose that the rooms are rectangular.
// However, I suppose that if I arrive from A to B using a direction,
// I can leave from B to A using the opposite direction.

class Int final {
private:
  int64_t mInt;

public:
  Int() noexcept = default;

  Int(int64_t aInt) noexcept : mInt(aInt) {
  }

  Int(std::string &aString) : mInt(std::stoll(aString)) {
  }

  int64_t toInt() const noexcept {
    return mInt;
  }

  operator int64_t() const noexcept {
    return mInt;
  }

  Int &operator=(int64_t const aInt) noexcept {
    mInt = aInt;
    return *this;
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
  Intcode() = default;

  Intcode(std::ifstream &aIn) {
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

  Intcode(Intcode const &aOther) = default;
  Intcode &operator=(Intcode const &aOther) = default;

  void input(tNumber const &aInput) noexcept {
    mInputs.push_back(aInput);
  }

  void input(int64_t const aInput) noexcept {
    mInputs.push_back(aInput);
  }

  void input(std::string const &aInput) noexcept {
    for(auto &i : aInput) {
      mInputs.push_back(static_cast<int64_t>(i));
    }
    mInputs.push_back(static_cast<int64_t>('\n'));
  }

  tNumber output() {
    return get(mOutputs);
  }

  std::string outputLine() {
    std::string line;
    while(hasOutput()) {
      int64_t ch = get(mOutputs).toInt();
      if(ch != '\n') {
        line.push_back(ch);
      }
      else if(line.size() > 0u) {
        break;
      }
      else { // nothing to do
      }  
    }
    return line;
  }

  void skipOutput() {
    mOutputs.clear();
  }
  
  bool hasOutput() {
    return !mOutputs.empty();
  }

  void printStatus() const noexcept {
    std::cout << "input: " << mInputs.size() << " output: " << mOutputs.size() << " memory: " << mMemory.size() << '\n';

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
        std::string message("PC: ");
        message += std::to_string(mProgramCounter);
        message += " mem: ";
        message += std::to_string(mMemory.size());
        throw std::invalid_argument(message);
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
      tNumber zero = 0;
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

struct Room final {
  static constexpr uint8_t  cDirNameLength = 6u;
  static constexpr uint8_t       cDirCount = 4u;
  static constexpr size_t      cPrefixSize = 2u;
  enum class Direction : uint8_t {
    cNorth   = 0u,
    cEast    = 1u,
    cSouth   = 2u,
    cWest    = 3u,
    cIllegal = 4u
  };
  static constexpr Direction cOppositeDir[]              = { Direction::cSouth, Direction::cWest, Direction::cNorth, Direction::cEast, Direction::cIllegal };
  static constexpr char      cWall[]                     = "wall";
  static constexpr char      cNowhere[]                  = "nowhere";
  static constexpr char      cNotVisited[]               = "not visited";
  static constexpr char      cDirNames[][cDirNameLength] = { "north", "east", "south", "west", "" };

  std::array<std::string, cDirCount> doors;        // wall, not visited or [room name]
  std::unordered_set<std::string>    objects;
  std::string                        cameFrom;     // direction name
  std::string                        sensitiveDir;

  Room(std::string const &aPreviousName, std::string const &aDir2reachHere) noexcept : doors({cWall, cWall, cWall, cWall}) {
    size_t opposite = static_cast<size_t>(cOppositeDir[find(aDir2reachHere)]);
    cameFrom = cDirNames[opposite];
    if(aPreviousName != cNowhere) {
      doors[opposite] = aPreviousName;
    }
    else { // nothing to do
    }
  }

  void setDoor(std::string const &aDoor) {
    for(size_t i = 0; i < cDirCount; ++i) {
      if(aDoor.find(cDirNames[i]) == cPrefixSize && doors[i] == cWall) {
        doors[i] = cNotVisited;
      }
      else { // nothing to do
      }
    }
  }

  void setDoor(std::string const &aTargetRoom, std::string const &aDirection) {
    size_t i = find(aDirection);
    doors[i] = aTargetRoom;
  }

  std::string getNextDir() noexcept {
    size_t i;
    size_t fromHere = cDirCount;
    for(i = 0u; i < cDirCount; ++i) {
      if(doors[i] == cNotVisited) {
        break;
      }
      else { // nothing to do
      }
    }
    std::string result;
    if(i < cDirCount) {
      result = cDirNames[i];
    }
    else {
      result = cameFrom;
    }
    return result;
  }

private:
  size_t find(std::string const &aDirection) {
    size_t i;
    for(i = 0u; i < cDirCount; ++i) {
      if(aDirection == cDirNames[i]) {
        break;
      }
      else { // nothing to do
      }
    }
    return i;
  }
};

constexpr char Room::cWall[];
constexpr char Room::cNowhere[];
constexpr char Room::cNotVisited[];
constexpr char Room::cDirNames[][Room::cDirNameLength];

struct RoomResult final {
  std::string name;
  Room        room;
  bool        rejected;

  RoomResult(std::string const &aName, Room const &aRoom, bool const aRejected) noexcept
  : name(aName)
  , room(aRoom)
  , rejected(aRejected) {
  }
};

class Adventure final {
private:
/*  enum class CommandType : uint8_t {
    cMove = 0u,
    cTake = 1u,
    cDrop = 2u
  };*/
  static constexpr char   cMagicPossiblePrefix[] = "- ";
  static constexpr char   cMagicItems[]          = "Items here:";
  static constexpr char   cMagicCommand[]        = "Command?";
  static constexpr char   cSafe[]                = "safe";
  static constexpr char   cCommandTake[]         = "take ";
  static constexpr char   cCommandDrop[]         = "drop ";
  static constexpr char   cRejected[]            = "rejected";
  static constexpr char   cCheckpointName[]      = "== Security Checkpoint ==";
  static constexpr char   cSensitiveName[]      = "== Pressure-Sensitive Floor ==";
 
  Intcode<Int>                                 mComputer;
  std::unordered_map<std::string, Room>        mMap;
  std::unordered_map<std::string, std::string> mObjects;
  std::list<std::string>                       mSurveyPath;

public:
  Adventure(std::ifstream &aIn) : mComputer(aIn) {
    mComputer.start();
  }

  void gatherObjects() {
    bool foundSecurity = false;
    mComputer.run();
    RoomResult start = handleRoom(Room::cNowhere, Room::cDirNames[Room::cDirCount]);
    std::pair<std::string, Room> toInsert(start.name, start.room);
    auto here = mMap.insert(toInsert).first; // insert: first is iterator, second is bool
    mObjects["giant electromagnet"] = "cheat"; // this causes problem only after move, which would require a more complicated logic
    mObjects["infinite loop"] = "cheat";
    while(true) {
      gatherObjects(here->second);
      std::string direction = here->second.getNextDir();
      if(direction.size() == 0u) {
        break;         // finished surveying the dungeons and gathering safe objects
      }
      else { // nothing to do
      }
      if(!foundSecurity) {
        mSurveyPath.push_back(direction);
      }
      else { // nothing to do
      }
      mComputer.input(direction);
      mComputer.run();
      RoomResult roomCandidate = handleRoom(here->first, direction);
      here->second.setDoor(roomCandidate.name, direction);
      if(roomCandidate.name == cCheckpointName) {
        foundSecurity = true;
      }
      else { // nothing to do
      }
      if(!roomCandidate.rejected) {
        auto next = mMap.find(roomCandidate.name);
        if(next == mMap.end()) {
          std::pair<std::string, Room> toInsert(roomCandidate.name, roomCandidate.room);
          here = mMap.insert(toInsert).first;
        }
        else {
          here = next;
        }
      }
      else {
        here->second.sensitiveDir = direction;
      }
    }
  }

  std::string obtainCode() {
    std::string result;
    std::vector<bool> hand(mObjects.size(), true);
    std::vector<std::string> objects;
    objects.reserve(mObjects.size());
    for(auto &i : mObjects) {
      if(i.second == cSafe) {
        objects.push_back(i.first);
      }
      else { // nothing to do
      }
    }
    for(auto &dir : mSurveyPath) {
      mComputer.input(dir);
      mComputer.run();
      mComputer.skipOutput();
    }
    auto here = mMap.find(cCheckpointName);
    uint64_t bitCount = objects.size();
    uint64_t grayEnd = 0xaaaaaaaaaaaaaaaaull >> (bitCount % 2ull);
    uint64_t grayMask = (1ull << bitCount) - 1ull;
    grayEnd &= grayMask;
    for(uint64_t i = grayEnd + 1ull; i != grayEnd; i = (i + 1ull) & grayMask) {
      uint64_t gray = i ^ (i >> 1ull);
      std::string command;
      uint64_t bit;
      for(bit = 0u; bit < bitCount; ++bit) {
        if(hand[bit] && (gray & 1ull << bit) == 0ull) {
          command = cCommandDrop;
          hand[bit] = false;
          break;
        }
        else
        if(!hand[bit] && (gray & 1ull << bit) != 0ull) {
          command = cCommandTake;
          hand[bit] = true;
          break;
        }
        else { // nothing to do
        }
      }
      command += objects[bit];
      mComputer.input(command);
      mComputer.run();
      mComputer.skipOutput();
      mComputer.input(here->second.sensitiveDir);
      mComputer.run();
      std::string lastLine;
      while(mComputer.hasOutput()) {
        lastLine = mComputer.outputLine();
      }
      if(lastLine != cMagicCommand) {
        result = lastLine;
        break;
      }
      else { // nothing to do
      }
    }
    return result;
  }

private:
  RoomResult handleRoom(std::string const &aPreviousName, std::string const &aDir2reachHere) {
    std::string line = mComputer.outputLine(); // room name
    RoomResult result(line, Room(aPreviousName, aDir2reachHere), false);
    line = mComputer.outputLine();             // description
    line = mComputer.outputLine();             // cMagicDoors
    while(true) {
      line = mComputer.outputLine();
      if(line.find(cMagicPossiblePrefix) == 0u) {
        result.room.setDoor(line);
      }
      else {
        break;
      }
    }
    if(line == cMagicItems) {
      while(true) {
        line = mComputer.outputLine();
        if(line.find(cMagicPossiblePrefix) == 0u) {
          result.room.objects.insert(line.substr(Room::cPrefixSize));
        }
        else {
          break;
        }
      }
    }
    else { // nothing to do
    }
    if(line != cMagicCommand) {
      result.rejected = true;
    }
    else { // nothing to do
    }
    return result;
  }

  void gatherObjects(Room &aRoom) {
    for(auto i = aRoom.objects.begin(); i != aRoom.objects.end();) {
      if(mObjects.find(*i) == mObjects.end()) {
        Intcode<Int> risky = mComputer;
        std::string command(cCommandTake);
        command += *i;
        risky.input(command);
        risky.run();
        std::string line = risky.outputLine(); // You take the...
        line = risky.outputLine();
        if(line == cMagicCommand) {
          mComputer = risky;
          mObjects[*i] = cSafe;
          i = aRoom.objects.erase(i);
        }
        else {
          mObjects[*i] = line; // danger
          ++i;
        }
      }
      else { // dangerous, leave there
        ++i;
      }
    }
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
    
    Adventure adventure(in);
    auto begin = std::chrono::high_resolution_clock::now();
    adventure.gatherObjects();
    std::string result = adventure.obtainCode();
    auto end = std::chrono::high_resolution_clock::now();
    auto timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
    std::cout << "duration: " << timeSpan.count() << '\n';
    std::cout << result << '\n';
  }
  catch(std::exception const &e) {
    std::cerr << "fail: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
