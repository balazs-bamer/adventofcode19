#include <map>
#include <list>
#include <deque>
#include <cmath>
#include <chrono>
#include <limits>
#include <vector>
#include <iomanip>
#include <utility>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>

// Solves all the examples but way too slow on the real input.

class Coordinates final {
private:
  static uint64_t constexpr cShiftX = 32u;
  static uint64_t constexpr cShiftY = 48u;
  static uint64_t constexpr cMask   = 0xffffu;
  static int constexpr cDeltaX[]    = { 0, 1, 0, -1};
  static int constexpr cDeltaY[]    = {-1, 0,  1, 0};

  int mX = 0;
  int mY = 0;

public:
  Coordinates() noexcept : mX(0), mY(0) {
  }

  Coordinates(int const aX, int const aY) noexcept : mX(aX), mY(aY) {
  }

  Coordinates(uint64_t const aCompact) noexcept : mX((aCompact >> cShiftX) & cMask), mY((aCompact >> cShiftY) & cMask) {
  }

  Coordinates& operator=(uint64_t const aCompact) noexcept {
    mX = (aCompact >> cShiftX) & cMask;
    mY = (aCompact >> cShiftY) & cMask;
    return *this;
  }

  Coordinates operator+(int const aDirection) const noexcept {
    return Coordinates(mX + cDeltaX[aDirection] , mY + cDeltaY[aDirection]);
  }

  Coordinates& operator+=(int const aDirection) noexcept {
    mX += cDeltaX[aDirection];
    mY += cDeltaY[aDirection];
    return *this;
  }

  int getX() const noexcept {
    return mX;
  }

  int getY() const noexcept {
    return mY;
  }

  bool isOrigin() const noexcept {
    return mX == 0 && mY == 0;
  }

  bool operator==(Coordinates const &aOther) const noexcept {
    return mX == aOther.mX && mY == aOther.mY;
  }

  bool operator!=(Coordinates const &aOther) const noexcept {
    return mX != aOther.mX || mY != aOther.mY;
  }

  explicit operator uint64_t() const noexcept {
    return (static_cast<uint64_t>(mX) & cMask) << cShiftX | (static_cast<uint64_t>(mY) & cMask) << cShiftY;
  }
};

int constexpr Coordinates::cDeltaX[];
int constexpr Coordinates::cDeltaY[];

template<>
struct std::hash<Coordinates> {
  size_t operator()(Coordinates const &aKey) const {
    return std::hash<int>{}(aKey.getX()) ^ (std::hash<int>{}(aKey.getY()) << 1u);
  }
};
  
class Node {
public:
  static size_t constexpr cNumRobots = 4u; 

private:
  uint32_t mKeys = 0u;
  std::array<Coordinates, cNumRobots> mPositions;

public:
  Node() noexcept = default;
  
  Node(int const aX, int const aY) noexcept {
    Coordinates start(aX, aY);
    for(size_t i = 0u; i < cNumRobots; ++i) {
      mPositions[i] = start + i;
      mPositions[i] += (i + 3u) % cNumRobots;
    }
  }

  int getX(size_t const aRobot) const noexcept {
    return mPositions[aRobot].getX();
  }

  int getY(size_t const aRobot) const noexcept {
    return mPositions[aRobot].getY();
  }

  Coordinates const &getPosition(size_t const aRobot) const noexcept {
    return mPositions[aRobot];
  }

  uint32_t getKeys() const noexcept {
    return mKeys;
  }

  bool operator==(Node const &aOther) const noexcept {
    return mKeys == aOther.mKeys && mPositions == aOther.mPositions;
  }

  Node &operator|=(uint32_t const aKey) noexcept {
    mKeys |= aKey;
    return *this;
  }

  void move(size_t const aRobot, int const aDir) noexcept {
    mPositions[aRobot] += aDir;
  }

void print() {
  for(int i = 0; i < 4; ++i)
    std::cout << mPositions[i].getX() << ':' << mPositions[i].getY() << ' ';
std::cout << mKeys << '\n';
}
};

template<>
struct std::hash<Node> {
  size_t operator()(Node const &aKey) const {
    size_t result = std::hash<uint32_t>{}(aKey.getKeys());
    for(size_t i = 0u; i < Node::cNumRobots; ++i) {
      result ^= (std::hash<Coordinates>{}(aKey.getPosition(i)) << (i + 1u));
    }
    return result;
  }
};

class Labyrinth final {
private:
  static size_t   constexpr cProgressInterval = 100000u;
  static char     constexpr cCompactOffset    = '@';
  static char     constexpr cCompactCorridor  = '^' - cCompactOffset;
  static char     constexpr cCompactWall      = '[' - cCompactOffset;
  static char     constexpr cCompactStart     =  0;
  static char     constexpr cCompactDoorStart = 'A' - cCompactOffset;
  static char     constexpr cCompactDoorEnd   = cCompactDoorStart + 'Z' - 'A' + 1;
  static char     constexpr cCompactKeyStart  = 'a' - cCompactOffset;
  static char     constexpr cCompactKeyEnd    = cCompactKeyStart + 'z' - 'a' + 1;
  static char     constexpr cCompactDoor2key  = 'a' - 'A';
  static char     constexpr cNewLine          = '\n';
  static char     constexpr cStart            = '@';
  static char     constexpr cCorridor         = '.';
  static char     constexpr cWall             = '#';
  static int      constexpr cDirCount         =  4;
  static size_t   constexpr cInvalidResult    = std::numeric_limits<size_t>::max();
  static size_t   constexpr cMatrixSize       = cCompactOffset;

  int                                                      mStartX  = 0;
  int                                                      mStartY  = 0;
  std::deque<std::deque<char>>                             mMap;
  uint32_t                                                 mKeyMask = 0u;            // available keys

public:
  Labyrinth(std::ifstream &aIn) {
    bool newLine = true;
    bool foundStart = false;
    while(true) {
      if(newLine) {
        mMap.emplace_back(std::deque<char>());
        newLine = false;
      }
      else { // nothing to do
      }
      char input = aIn.get();
      if(input == cNewLine) {
        newLine = true;
        if(!foundStart) {
          ++mStartY;
          mStartX = 0;
        }
        else { // nothing to do
        }
      }
      else {
        if(input == cStart) {
          foundStart = true;
          input = cCompactStart;
        }
        else {
          mStartX += (foundStart ? 0 : 1);
          if(input == cWall) {
            input = cCompactWall;
          }
          else if(input == cCorridor) {
            input = cCompactCorridor;
          }
          else {
            input -= cCompactOffset;
            if(input >= cCompactKeyStart) {
              mKeyMask |= static_cast<uint32_t>(1u) << (input - cCompactKeyStart);
            }
            else { // nothing to do
            }
          }
        }
        mMap.back().push_back(input);
      }
      if(!aIn.good()) {
        break;
      }
      else { // nothing to do
      }
    }
    while(mMap.back().size() <= 1u) {
      mMap.pop_back();
    }
    mMap[mStartY][mStartX] = cCompactWall;
    mMap[mStartY + 1][mStartX] = cCompactWall;
    mMap[mStartY - 1][mStartX] = cCompactWall;
    mMap[mStartY][mStartX + 1] = cCompactWall;
    mMap[mStartY][mStartX - 1] = cCompactWall;
  }

  size_t calculateShortestPath() {
    size_t iterations = 0u;
    size_t result = cInvalidResult;
    Node lastNode;
    Node start(mStartX, mStartY);
    std::unordered_map<Node, size_t> costs; // assume cInvalidResult for nodes not present here
    std::multimap<size_t, Node> queue;
    costs[start] = 0u;
    queue.insert(std::pair<size_t, Node>(0u, start));
std::cout << mKeyMask << '\n';
    while(!queue.empty()) {
      auto smallest = queue.begin();
      size_t smallestCost = smallest->first;
      Node smallestNode = smallest->second;
      queue.erase(smallest);
      uint32_t keys = smallestNode.getKeys();
      if(iterations % cProgressInterval == 0u) {
        for(uint32_t i = 0u; i < 32u; ++i) {
          std::cout << (((keys << i) & (1u << 31u)) == 0u ? '0' : '1');
        }
        std::cout << ' ' << iterations << '\n';
      }
      else { // nothing to do
      }
      if(keys == mKeyMask) {
        if(smallestCost < result) {
          result = smallestCost;
          lastNode = smallestNode;
        }
        else { // nothing to do
        }
      }
      else { // nothing to do
      }
      for(size_t robot = 0u; robot < Node::cNumRobots; ++robot) {
        for(size_t i = 0u; i < cDirCount; ++i) {
          Node otherNode = smallestNode;
          otherNode.move(robot, i);
          char found = mMap[otherNode.getY(robot)][otherNode.getX(robot)];
          if(found == cCompactCorridor || (found >= cCompactKeyStart && found < cCompactKeyEnd)
           || (found >= cCompactDoorStart && found < cCompactDoorEnd && ((smallestNode.getKeys() & (1u << (found - cCompactDoorStart))) != 0u))) {
            otherNode |= ((found >= cCompactKeyStart && found < cCompactKeyEnd) ? 1u << (found - cCompactKeyStart) : 0u);
            auto other = costs.find(otherNode);
            size_t otherCost = (other == costs.end() ? cInvalidResult : other->second);
            size_t newCost = smallestCost + 1u;
            if(newCost < otherCost) {
              if(other == costs.end()) {
                costs[otherNode] = newCost;
              }
              else {
                other->second = newCost;
              }
              queue.insert(std::pair<size_t, Node>(newCost, otherNode));
            }
            else { // nothing to do
            }
          }
          else { // nothing to do
          }
        }
      }
      ++iterations;
    }
std::cout << "iter: " << iterations << '\n';
    return result;
  }
};
  
size_t constexpr Labyrinth::cInvalidResult;

int main(int const argc, char **argv) {
  try {
    if(argc == 1) {
      throw std::invalid_argument("Need input filename.");
    }
    else { // nothing to do
    }
    std::ifstream in(argv[1]);
    Labyrinth labyrinth(in);
    auto begin = std::chrono::high_resolution_clock::now();
    size_t length = labyrinth.calculateShortestPath();
    auto end = std::chrono::high_resolution_clock::now();
    auto timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
    std::cout << "duration: " << timeSpan.count() << '\n';
    std::cout << length << '\n';
  }
  catch(std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
