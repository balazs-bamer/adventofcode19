#include <map>
#include <list>
#include <deque>
#include <cmath>
#include <chrono>
#include <limits>
#include <vector>
#include <utility>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>

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
  

class Labyrinth final {
private:
  static char     constexpr cCompactOffset    = '@';
  static char     constexpr cCompactCorridor  = '^';
  static char     constexpr cCompactWall      = '[';
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
  uint64_t                                                 mKeyMask = 0u;            // available keys

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
              mKeyMask |= static_cast<uint64_t>(1u) << (input - cCompactKeyStart);
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
  }

  size_t calculateShortestPath() {
    size_t iterations = 0u;
    size_t result = cInvalidResult;
    uint64_t lastNode;
    Coordinates start(mStartX, mStartY);
    std::unordered_map<uint64_t, size_t> costs; // assume cInvalidResult for nodes not present here
    std::multimap<size_t, uint64_t> queue;
    std::unordered_map<uint64_t, uint64_t> previous;
    uint64_t startValue = static_cast<uint64_t>(start);
    costs[startValue] = 0u;
    queue.insert(std::pair<size_t, uint64_t>(0u, startValue));
    previous[startValue] = std::numeric_limits<uint64_t>::max();
    while(!queue.empty()) {
      auto smallest = queue.begin();
      size_t smallestCost = smallest->first;
      uint64_t smallestNode = smallest->second;
      queue.erase(smallest);
      if((smallestNode & mKeyMask) == mKeyMask) {
        if(smallestCost < result) {
          result = smallestCost;
          lastNode = smallestNode;
        }
        else { // nothing to do
        }
      }
      else { // nothing to do
      }
      Coordinates smallestCoord(smallestNode);
      for(size_t i = 0; i < cDirCount; ++i) {
        Coordinates otherCoord = smallestCoord + i;
        char found = mMap[otherCoord.getY()][otherCoord.getX()];
        if(found != cCompactWall && (found >= cCompactKeyStart || found == cCompactStart || (smallestNode & (1u << (found - cCompactDoorStart))) != 0u)) {
          uint64_t otherNode = (static_cast<uint64_t>(otherCoord) | (smallestNode & mKeyMask) | (found >= cCompactKeyStart ? 1u << (found - cCompactKeyStart) : 0u));
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
            queue.insert(std::pair<size_t, uint64_t>(newCost, otherNode));
            previous[otherNode] = smallestNode;
          }
          else { // nothing to do
          }
        }
        else { // nothing to do
        }
      }
      ++iterations;
    }
std::cout << "iter: " << iterations << '\n';
    for(uint64_t node = lastNode; node != std::numeric_limits<uint64_t>::max(); node = previous[node]) {
      Coordinates coord = node;
      char found = mMap[coord.getY()][coord.getX()];
      if(found != cCompactCorridor) {
        std::cout << (char)(cCompactOffset + found) << ' ';
      }
      else { // nothing to do
      }
    }
std::cout << '\n';
    return result;
  }

private:
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
