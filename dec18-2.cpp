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

// Computes the real challange without the trick with theoretical flaws used on Reddit.

class Coordinates final {
private:
  static int constexpr cDeltaX[]    = { 0, 1, 0, -1};
  static int constexpr cDeltaY[]    = {-1, 0,  1, 0};

  int mX = 0;
  int mY = 0;

public:
  Coordinates() noexcept : mX(0), mY(0) {
  }

  Coordinates(int const aX, int const aY) noexcept : mX(aX), mY(aY) {
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
private:
  static size_t   constexpr cDirCount =   4u;
  static uint64_t constexpr cLocShift =  32u;
  static uint64_t constexpr cByteSize =   8u;
  static uint64_t constexpr cByteMask = 255u;
  static uint64_t constexpr cKeyMask  = (static_cast<uint64_t>(1u) << cLocShift) - 1u;

  uint64_t mValue = 0u;

public:
  Node() noexcept = default;

  uint64_t getValue() const noexcept {
    return mValue;
  }

  size_t getLoc(size_t const aWhich) const noexcept {
    return (mValue >> (cLocShift + aWhich * cByteSize)) & cByteMask;
  }

  uint32_t getKeys() const noexcept {
    return mValue & cKeyMask;
  }

  bool operator==(Node const &aOther) const noexcept {
    return mValue == aOther.mValue;
  }

  Node &operator|=(uint32_t const aKey) noexcept {
    mValue |= aKey;
    return *this;
  }

  void move(size_t const aWhich, char const aNewLoc) noexcept {
    uint64_t shift = cLocShift + aWhich * cByteSize;
    uint64_t mask = ~(cByteMask << shift);
    uint64_t byte = static_cast<uint64_t>(aNewLoc);
    mValue = (mValue & mask) | (byte << shift);
  }
};

template<>
struct std::hash<Node> {
  size_t operator()(Node const &aKey) const {
    return std::hash<uint64_t>{}(aKey.getValue());
  }
};
  
char   constexpr cCompactOffset    = '@';
char   constexpr cCompactCorridor  = '^';
char   constexpr cCompactWall      = '[';
char   constexpr cCompactStart     =  0;
char   constexpr cCompactDoorStart = 'A' - cCompactOffset;
char   constexpr cCompactDoorEnd   = cCompactDoorStart + 'Z' - 'A' + 1;
char   constexpr cCompactKeyStart  = 'a' - cCompactOffset;
char   constexpr cCompactKeyEnd    = cCompactKeyStart + 'z' - 'a' + 1;
char   constexpr cCompactDoor2key  = 'a' - 'A';
char   constexpr cCharShift        = cCompactKeyEnd - cCompactKeyStart;

class Labyrinth final {
private:
  static size_t constexpr cProgressInterval = 10000u;
  static char   constexpr cNewLine          = '\n';
  static char   constexpr cStart            = '@';
  static char   constexpr cCorridor         = '.';
  static char   constexpr cWall             = '#';
  static int    constexpr cDirCount         =  4;
  static size_t constexpr cInvalidResult    = std::numeric_limits<size_t>::max();
  static size_t constexpr cMatrixSize       = cCompactOffset;

  std::array<Coordinates, cDirCount>                                              mStart; 
  std::deque<std::deque<char>>                                                    mMap;
  std::array<std::array<std::array<size_t, cMatrixSize>, cMatrixSize>, cDirCount> mDistances;        // distances from outer index to inner index
  uint64_t                                                                        mKeyMask = 0u;            // available doors

public:
  Labyrinth(std::ifstream &aIn) {
    bool newLine = true;
    bool foundStart = false;
    int startX = 0;
    int startY = 0;
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
          ++startY;
          startX = 0;
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
          startX += (foundStart ? 0 : 1);
          if(input == cWall) {
            input = cCompactWall;
          }
          else if(input == cCorridor) {
            input = cCompactCorridor;
          }
          else {
            input -= cCompactOffset;
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
    Coordinates start(startX, startY);
    for(size_t i = 0u; i < cDirCount; ++i) {
      Coordinates here = start + i;
      mMap[here.getY()][here.getX()] = cCompactWall;
      here += (i + 3u) % cDirCount;
      mMap[here.getY()][here.getX()] = cCompactStart;
      mStart[i] = here;
    }
  }

  void buildDistanceMatrix() {
    for(size_t robot = 0u; robot < cDirCount; ++robot) {
      std::vector<std::vector<size_t>>      distanceMap;      // required steps from start
      std::unordered_map<char, Coordinates> locations;
      buildDistanceMap(robot, distanceMap, locations);
      {
        std::array<size_t, cMatrixSize> other;
        std::fill(other.begin(), other.end(), cInvalidResult);
        std::fill(mDistances[robot].begin(), mDistances[robot].end(), other);
      }
      for(auto &i : locations) {
        char compactX = i.first; // to
        mKeyMask |= (compactX >= cCompactKeyStart ? (1u << (i.first - cCompactKeyStart)) : 0u);
        for(auto &j : locations) {
          char compactY = j.first; // from
          std::deque<char> visitedForward;
          std::deque<char> visitedBackward;
          Coordinates locI = i.second;
          Coordinates locJ = j.second;
          size_t headI = gatherHead(distanceMap, locI, distanceMap[locJ.getY()][locJ.getX()], visitedForward);
          size_t headJ = gatherHead(distanceMap, locJ, distanceMap[locI.getY()][locI.getX()], visitedBackward);
          size_t distance = headI + headJ + gatherTogether(distanceMap, locI, locJ, visitedForward, visitedBackward);
          if(compactX != compactY && ((visitedForward.empty() && visitedBackward.empty())
            || (!visitedForward.empty() && visitedForward.front() == compactY)
            || (!visitedBackward.empty() && visitedBackward.front() == compactX))) {
            mDistances[robot][compactX][compactY] = distance;
          }
          else { // nothing to do
          }
        }
      }
    }
  }

  size_t calculateShortestPath() {
    size_t iterations = 0u;
    size_t result = cInvalidResult;
    Node lastNode;
    Node start;
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
      for(size_t robot = 0u; robot < cDirCount; ++robot) {
        auto &line = mDistances[robot][smallestNode.getLoc(robot)];
        for(size_t i = 0; i < cMatrixSize; ++i) {
          Node otherNode = smallestNode;
          otherNode.move(robot, i);
          if(line[i] != cInvalidResult && (i >= cCompactKeyStart || i == cCompactStart || (smallestNode.getKeys() & (1u << (i - cCompactDoorStart))) != 0u)) {
            otherNode |= ((i >= cCompactKeyStart && i < cCompactKeyEnd) ? 1u << (i - cCompactKeyStart) : 0u);
            auto other = costs.find(otherNode);
            size_t otherCost = (other == costs.end() ? cInvalidResult : other->second);
            size_t newCost = smallestCost + line[i];
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

private:
  size_t gatherHead(std::vector<std::vector<size_t>> const &aDistanceMap, Coordinates &aHead, size_t const aOtherDistance, std::deque<char> &aVisited) const {
    size_t result = 0u;
    size_t now = aDistanceMap[aHead.getY()][aHead.getX()];
    while(now > aOtherDistance) {
      --now;
      step(aDistanceMap, aHead, now, aVisited);
      ++result;
    }
    return result;
  }

  size_t gatherTogether(
      std::vector<std::vector<size_t>> const &aDistanceMap, Coordinates const aLoc1, Coordinates const aLoc2, std::deque<char> &aVisited1, std::deque<char> &aVisited2) const {
    Coordinates loc1 = aLoc1;
    Coordinates loc2 = aLoc2;
    size_t result = 0u;
    size_t target = aDistanceMap[loc1.getY()][loc1.getX()];
    while(loc1 != loc2) {
      --target;
      step(aDistanceMap, loc1, target, aVisited1);
      step(aDistanceMap, loc2, target, aVisited2);
      result += 2u;
    }
    return result;
  }

  void step(
      std::vector<std::vector<size_t>> const &aDistanceMap, Coordinates &aHead, size_t const aTarget, std::deque<char> &aVisited) const {
    Coordinates look;
    for(int i = 0u; i < cDirCount; ++i) {
      look = aHead + i;
      if(aDistanceMap[look.getY()][look.getX()] == aTarget) {
        break;
      }
      else { // nothing to do
      }
    }
    aHead = look;
    char what = mMap[aHead.getY()][aHead.getX()];
    if(what != cCompactCorridor) {
      aVisited.push_back(what);
    }
    else { // nothing to do
    }
  }
  
  void buildDistanceMap(size_t const aWhich, std::vector<std::vector<size_t>> &aDistanceMap, std::unordered_map<char, Coordinates> &aLocations) {
    {
      std::vector<size_t> line(mMap.front().size(), cInvalidResult);
      aDistanceMap = std::vector<std::vector<size_t>>(mMap.size(), line);
    }
    size_t alias = 0u;
    std::list<Coordinates> toVisit;
    std::list<size_t>      distances;
    Coordinates here = mStart[aWhich];
    toVisit.push_back(here);
    aLocations[cCompactStart] = here;
    distances.emplace_back(0u);
    while(!toVisit.empty()) {
      here = toVisit.front();
      toVisit.pop_front();
      size_t distance = distances.front();
      distances.pop_front();
      if(aDistanceMap[here.getY()][here.getX()] < cInvalidResult) {
        continue;
      }
      else { // nothing to do
      }
      char what = mMap[here.getY()][here.getX()];
      if(what != cCompactCorridor) {
        aLocations[what] = here;
        ++alias;
      }
      else { // nothing to do
      }
      aDistanceMap[here.getY()][here.getX()] = distance;
      ++distance;
      for(int i = 0u; i < cDirCount; ++i) {
        Coordinates look = here + i;
        if(mMap[look.getY()][look.getX()] != cCompactWall && aDistanceMap[look.getY()][look.getX()] == cInvalidResult) {
          toVisit.push_back(look);
          distances.push_back(distance);
        }
        else { // nothing to do
        }
      }
    }
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
    labyrinth.buildDistanceMatrix();
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
