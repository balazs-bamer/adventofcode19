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

// Could be very efficient, finishes all the examples, but fails on the real input.

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
  static char   constexpr cNewLine        = '\n';
  static char   constexpr cStart          = '@';
  static char   constexpr cCorridor       = '.';
  static char   constexpr cWall           = '#';
  static int    constexpr cDirCount       =  4;
  static size_t constexpr cInvalidResult  = std::numeric_limits<size_t>::max();
  static size_t constexpr cMatrixSize     = cCompactOffset;

  int                                                      mStartX  = 0;
  int                                                      mStartY  = 0;
  std::deque<std::deque<char>>                             mMap;
  std::vector<std::vector<size_t>>                         mDistanceMap;      // required steps from start
  std::unordered_map<char, Coordinates>                    mLocations;
  std::array<std::array<size_t, cMatrixSize>, cMatrixSize> mDistances;        // distances from outer index to inner index
  uint64_t                                                 mKeyMask = 0u;            // available doors

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

  void buildDistanceMap() {
    {
      std::vector<size_t> line(mMap.front().size(), cInvalidResult);
      mDistanceMap = std::vector<std::vector<size_t>>(mMap.size(), line);
    }
    size_t alias = 0u;
    std::list<Coordinates> toVisit;
    std::list<size_t>      distances;
    Coordinates here(mStartX, mStartY);
    toVisit.push_back(here);
    mLocations[cCompactStart] = here;
    distances.emplace_back(0u);
    while(!toVisit.empty()) {
      here = toVisit.front();
      toVisit.pop_front();
      size_t distance = distances.front();
      distances.pop_front();
      if(mDistanceMap[here.getY()][here.getX()] < cInvalidResult) {
        continue;
      }
      else { // nothing to do
      }
      char what = mMap[here.getY()][here.getX()];
      if(what != cCompactCorridor) {
        mLocations[what] = here;
        ++alias;
      }
      else { // nothing to do
      }
      mDistanceMap[here.getY()][here.getX()] = distance;
      ++distance;
      for(int i = 0u; i < cDirCount; ++i) {
        Coordinates look = here + i;
        if(mMap[look.getY()][look.getX()] != cCompactWall && mDistanceMap[look.getY()][look.getX()] == cInvalidResult) {
          toVisit.push_back(look);
          distances.push_back(distance);
        }
        else { // nothing to do
        }
      }
    }
  }

  void buildDistanceMatrix() {
    {
      std::array<size_t, cMatrixSize> other;
      std::fill(other.begin(), other.end(), cInvalidResult);
      std::fill(mDistances.begin(), mDistances.end(), other);
    }
    for(auto &i : mLocations) {
      char compactX = i.first; // to
      mKeyMask |= (compactX >= cCompactKeyStart ? (1u << (i.first - cCompactKeyStart)) : 0u);
      for(auto &j : mLocations) {
        char compactY = j.first; // from
        std::deque<char> visitedForward;
        std::deque<char> visitedBackward;
        Coordinates locI = i.second;
        Coordinates locJ = j.second;
        size_t headI = gatherHead(locI, mDistanceMap[locJ.getY()][locJ.getX()], visitedForward);
        size_t headJ = gatherHead(locJ, mDistanceMap[locI.getY()][locI.getX()], visitedBackward);
        size_t distance = headI + headJ + gatherTogether(locI, locJ, visitedForward, visitedBackward);
        if(compactX != compactY && (visitedForward.empty() && visitedBackward.empty() 
          || !visitedForward.empty() && visitedForward.front() == compactY
          || !visitedBackward.empty() && visitedBackward.front() == compactX)) {
          mDistances[compactX][compactY] = distance;
        }
        else { // nothing to do
        }
      }
    }
  }

  size_t calculateShortestPath() {
    size_t iterations = 0u;
    size_t result = cInvalidResult;
    uint64_t lastNode;
    std::unordered_map<uint64_t, size_t> costs; // assume cInvalidResult for nodes not present here
    std::multimap<size_t, uint64_t> queue;
    std::unordered_map<uint64_t, uint64_t> previous;
    costs[cCompactStart] = 0u;
    queue.insert(std::pair<size_t, uint64_t>(0u, cCompactStart));
    previous[cCompactStart] = std::numeric_limits<uint64_t>::max();
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
      auto &line = mDistances[smallestNode >> cCharShift];
      for(size_t i = 0; i < cMatrixSize; ++i) {
        uint64_t otherNode = ((i << cCharShift) | (smallestNode & mKeyMask) | (i >= cCompactKeyStart ? 1u << (i - cCompactKeyStart) : 0u));
        if(line[i] != cInvalidResult && (i >= cCompactKeyStart || i == cCompactStart || (smallestNode & (1u << (i - cCompactDoorStart))) != 0u)) {
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
      std::cout << (char)(64+(node >> cCharShift)) << ' ';
    }
std::cout << '\n';
    return result;
  }

private:
  size_t gatherHead(Coordinates &aHead, size_t const aOtherDistance, std::deque<char> &aVisited) const {
    size_t result = 0u;
    size_t now = mDistanceMap[aHead.getY()][aHead.getX()];
    while(now > aOtherDistance) {
      --now;
      step(aHead, now, aVisited);
      ++result;
    }
    return result;
  }

  size_t gatherTogether(Coordinates const aLoc1, Coordinates const aLoc2, std::deque<char> &aVisited1, std::deque<char> &aVisited2) const {
    Coordinates loc1 = aLoc1;
    Coordinates loc2 = aLoc2;
    size_t result = 0u;
    size_t target = mDistanceMap[loc1.getY()][loc1.getX()];
    while(loc1 != loc2) {
      --target;
      step(loc1, target, aVisited1);
      step(loc2, target, aVisited2);
      result += 2u;
    }
    return result;
  }

  void step(Coordinates &aHead, size_t const aTarget, std::deque<char> &aVisited) const {
    Coordinates look;
    for(int i = 0u; i < cDirCount; ++i) {
      look = aHead + i;
      if(mDistanceMap[look.getY()][look.getX()] == aTarget) {
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
    labyrinth.buildDistanceMap();
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
