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
#include <optional>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>

// Unfortunately the traveling salesman approach is not feasible,
// although the below solution examines 9,900,000 branches per second
// on my small Intel(R) Xeon(R) CPU E31220L @ 2.20GHz.
// I have an idea about cutting more branches but it is uncertain.

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
  
size_t constexpr cMatrixSize = 64u;

typedef std::array<bool, cMatrixSize> BoolArray;

struct Iteration final {
  static char   constexpr cCompactOffset    = '@';
  static char   constexpr cCompactKeyStart  = 'a' - cCompactOffset;
  static char   constexpr cCompactDoor2key  = 'a' - 'A';
  char                                  previous;
  size_t                                lengthSoFar = 0u;
  BoolArray                             was;
  BoolArray                             choices;
  BoolArray                             keysCollected;
  std::multimap<char, size_t>::iterator next;

  bool update(char const aWhat, BoolArray const &aDoors, std::array<std::vector<char>, cMatrixSize> const &aLocks) {
    bool result = true;
    choices[aWhat] = false;
    was[aWhat] = true;
    if(aWhat >= cCompactKeyStart) {
      char door = aWhat - cCompactDoor2key;
      choices[door] = aDoors[door];
      keysCollected[aWhat] = true;
    }
    else {
      for(auto i : aLocks[aWhat]) {
        if(!was[i]) {
          if(i >= cCompactKeyStart) {
            choices[i] = true;
          }
          else {
            if(keysCollected[i + cCompactDoor2key]) {
              choices[i] = true;
            }
            else { // nothing to do
              result = false;
            }
          }
        }
        else { // nothoing to do
        }
      }
    }
    return result;
  }
};

class Labyrinth final {
private:
  static char   constexpr cNewLine          = '\n';
  static char   constexpr cStart            = '@';
  static char   constexpr cCorridor         = '.';
  static char   constexpr cWall             = '#';
  static int    constexpr cDirCount         =  4;
  static char   constexpr cCompactOffset    = '@';
  static char   constexpr cCompactCorridor  = '^';
  static char   constexpr cCompactWall      = '[';
  static char   constexpr cCompactStart     =  0;
  static char   constexpr cCompactDoorStart = 'A' - cCompactOffset;
  static char   constexpr cCompactDoorEnd   = cCompactDoorStart + 'Z' - 'A' + 1;
  static char   constexpr cCompactKeyStart  = 'a' - cCompactOffset;
  static char   constexpr cCompactKeyEnd    = cCompactKeyStart + 'z' - 'a' + 1;
  static char   constexpr cCompactDoor2key  = 'a' - 'A';
  static char   constexpr cRecursionDepth   = cCompactDoorEnd - cCompactDoorStart + cCompactKeyEnd - cCompactKeyStart + 1u;

  int                                                                 mStartX  = 0;
  int                                                                 mStartY  = 0;
  std::deque<std::deque<char>>                                        mMap;
  std::vector<std::vector<size_t>>                                    mDistanceMap;
  std::unordered_map<char, Coordinates>                               mLocations;
  std::array<std::multimap<char, size_t>, cMatrixSize>                mDistances;
  std::array<std::array<std::vector<char>, cMatrixSize>, cMatrixSize> mVisitedLetters;
  BoolArray                                                           mDoors;
  std::array<std::vector<char>, cMatrixSize>                          mLocks;

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
      std::vector<size_t> line(mMap.front().size(), std::numeric_limits<size_t>::max());
      mDistanceMap = std::vector<std::vector<size_t>>(mMap.size(), line);
    }
    size_t alias = 0u;
    std::list<Coordinates> toVisit;
    std::list<size_t>      distances;
    toVisit.emplace_back(mStartX, mStartY);
    distances.emplace_back(0u);
    while(!toVisit.empty()) {
      Coordinates here = toVisit.front();
      toVisit.pop_front();
      size_t distance = distances.front();
      distances.pop_front();
      if(mDistanceMap[here.getY()][here.getX()] < std::numeric_limits<size_t>::max()) {
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
        if(mMap[look.getY()][look.getX()] != cCompactWall && mDistanceMap[look.getY()][look.getX()] == std::numeric_limits<size_t>::max()) {
          toVisit.push_back(look);
          distances.push_back(distance);
        }
        else { // nothing to do
        }
      }
    }
  }

  void buildDistanceMatrix() {
    std::fill(mDoors.begin(), mDoors.end(), false);
    for(auto &i : mLocations) {
      char compactX = i.first; // to
      addToLock(compactX);      
      mDoors[compactX] = (compactX >= cCompactDoorStart && compactX < cCompactDoorEnd);
      for(auto &j : mLocations) {
        char compactY = j.first; // from
        std::deque<char> visitedForward;
        std::deque<char> visitedBackward;
        Coordinates locI = i.second;
        Coordinates locJ = j.second;
        size_t headI = gatherHead(locI, mDistanceMap[locJ.getY()][locJ.getX()], visitedForward);
        size_t headJ = gatherHead(locJ, mDistanceMap[locI.getY()][locI.getX()], visitedBackward);
        size_t distance = headI + headJ + gatherTogether(locI, locJ, visitedForward, visitedBackward);
        mDistances[compactY].emplace(distance, compactX);
                                               // from      to
        auto &visitedLetters = mVisitedLetters[compactY][compactX];
        visitedLetters.reserve(visitedForward.size() + visitedBackward.size());
        std::for_each(visitedForward.begin(), visitedForward.end(), [&visitedLetters, compactX, compactY](auto i){
          if(i != compactX && i != compactY && i != cCompactStart) {
            visitedLetters.push_back(i);
          }
          else { // nothing to do
          }
        });
        std::for_each(visitedBackward.rbegin(), visitedBackward.rend(), [&visitedLetters, compactX, compactY](auto i){
          if(i != compactX && i != compactY && i != cCompactStart) {
            visitedLetters.push_back(i);
          }
          else { // nothing to do
          }
        });
      }
    }
/*for(char i = 0; i < 64; ++i) {
  if(mDoors[i]) {
    std::cout << (char)(i + 64) << ':';
    for(auto j : mLocks[i]) {
      std::cout << (char)(j + 64);
    }
    std::cout << '\n';
  }
}*/
  }

  size_t calculateShortestPath() {
    size_t iterations = 0u;
    size_t result = std::numeric_limits<size_t>::max();
    size_t pointer = 0u;
    std::array<Iteration, cRecursionDepth> stack;
    auto &bottom = stack[0u];
    bottom.previous = cCompactStart;
    bottom.lengthSoFar = 0u;
    std::fill(bottom.was.begin(), bottom.was.end(), false);
    std::fill(bottom.choices.begin(), bottom.choices.end(), false);
    std::fill(bottom.keysCollected.begin(), bottom.keysCollected.end(), false);
    bottom.update(cCompactStart, mDoors, mLocks);
    bottom.next = mDistances[cCompactStart].begin();
    while(bottom.next != mDistances[cCompactStart].end()) {
      auto &current = stack[pointer];
      if(std::count(current.choices.begin(), current.choices.end(), true) == 0u) { // end of recursion
        result = std::min<size_t>(result, current.lengthSoFar);
std::cout << "- result: " << result << ' ';
for(int i = 0; i <= pointer; ++i) std::cout << static_cast<char>(stack[i].previous + 64);
std::cout << '\n';
        --pointer;
        ++iterations;
      }
      else { // nothing to do
      }
      while(current.next != mDistances[current.previous].end() && !current.choices[current.next->second]) {
        current.next++;
      }
      if(current.next == mDistances[current.previous].end()) { // ran out of choices on this level
        --pointer;
        ++iterations;
        continue;
      }
      else { // nothing to do
      }
      size_t newLength = current.lengthSoFar + current.next->first;
      if(result <= newLength) { // this branch is definitely not better
        current.next++;
        ++iterations;
        continue;
      }
      else {
        char prePrevious = current.previous;
        char previous = current.next->second;
        current.next++;
        ++pointer;
        auto &next = stack[pointer];
        next.previous = previous;
        next.lengthSoFar = newLength;
        next.next = mDistances[previous].begin();
        next.was = current.was;
        next.choices = current.choices;
        next.keysCollected = current.keysCollected;
        next.update(previous, mDoors, mLocks);
        for(auto &i : mVisitedLetters[previous][prePrevious]) {
          if(!next.was[i] && ((i < cCompactDoorEnd && !next.keysCollected[i + cCompactDoor2key]) || !next.update(i, mDoors, mLocks))) { // stuck resolving this path, go further
            --pointer;
            current.next++;
            ++iterations;
            break;
          }
          else { // nothing to do
          }
        }
      }
    }
std::cout << "iterations: " << iterations << '\n';
    return result;
  }

private:
  void addToLock(char const aWhat) {
    auto here = mLocations[aWhat];
    std::deque<char> path;
    gatherHead(here, cCompactStart, path);
    path.push_back(cCompactStart);
    auto found = std::find_if(path.begin(), path.end(), [](auto i) {
      return ((i == cCompactStart) || (i >= cCompactDoorStart && i < cCompactDoorEnd));
    });
    if(aWhat != *found) {
      mLocks[*found].push_back(aWhat);
    }
  }

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
  
char constexpr Labyrinth::cCompactStart;

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
