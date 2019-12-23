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

struct Iteration final {
  static size_t constexpr cMatrixSize = 64u;

  char                                  previous;
  size_t                                lengthSoFar = 0u;
  std::array<bool, cMatrixSize>         choices;
  std::array<bool, cMatrixSize>         visited;
  std::multimap<char, size_t>::iterator next;
};

class Labyrinth final {
private:
  static char   constexpr cStart            = '@';
  static char   constexpr cCorridor         = '.';
  static char   constexpr cWall             = '#';
  static char   constexpr cNewLine          = '\n';
  static int    constexpr cDirCount         =  4;
  static size_t constexpr cMatrixSize       = Iteration::cMatrixSize;
  static char   constexpr cCompactOffset    = '@';
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
  std::array<bool, cMatrixSize>                                       mKeys;
  std::array<bool, cMatrixSize>                                       mDoors;

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
        }
        else {
          mStartX += (foundStart ? 0 : 1);
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
/*for(auto &line : mMap) {
std::cout << ch;
  }
std::cout << '\n';
}
std::cout << 'x' << mStartX << 'y' << mStartY << '\n';*/
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
      if(what != cCorridor) {
        mLocations[what] = here;
        ++alias;
      }
      else { // nothing to do
      }
      mDistanceMap[here.getY()][here.getX()] = distance;
      ++distance;
      for(int i = 0u; i < cDirCount; ++i) {
        Coordinates look = here + i;
        if(mMap[look.getY()][look.getX()] != cWall && mDistanceMap[look.getY()][look.getX()] == std::numeric_limits<size_t>::max()) {
          toVisit.push_back(look);
          distances.push_back(distance);
        }
        else { // nothing to do
        }
      }
    }
/*for(auto &i : mLocations) {
  std::cout << i.first << " x: " << i.second.getX() << ' ' << " y: " << i.second.getY() << ' ' << mDistanceMap[i.second.getY()][i.second.getX()] << '\n';
}*/
  }

  void buildDistanceMatrix() {
    std::fill(mKeys.begin(), mKeys.end(), false);
    std::fill(mDoors.begin(), mDoors.end(), false);
    for(auto &i : mLocations) {
      char shiftedI = i.first - cCompactOffset;
      mKeys[shiftedI] = (shiftedI >= cCompactKeyStart);
      mDoors[shiftedI] = (shiftedI >= cCompactDoorStart && shiftedI < cCompactDoorEnd);
      for(auto &j : mLocations) {
        char shiftedJ = j.first - cCompactOffset;
        std::deque<char> visitedForward;
        std::deque<char> visitedBackward;
        Coordinates locI = i.second;
        Coordinates locJ = j.second;
        size_t headI = gatherHead(locI, mDistanceMap[locJ.getY()][locJ.getX()], visitedForward);
        size_t headJ = gatherHead(locJ, mDistanceMap[locI.getY()][locI.getX()], visitedBackward);
        size_t distance = headI + headJ + gatherTogether(locI, locJ, visitedForward, visitedBackward);
        char compactX = j.first - cCompactOffset;  // to
        char compactY = i.first - cCompactOffset;  // from
        mDistances[compactY].emplace(distance, compactX);
                                               // from      to
        auto &visitedLetters = mVisitedLetters[compactY][compactX];
        visitedLetters.reserve(visitedForward.size() + visitedBackward.size());
        std::for_each(visitedForward.begin(), visitedForward.end(), [&visitedLetters, shiftedI, shiftedJ](auto i){
          if(i != shiftedI && i != shiftedJ) {
            visitedLetters.push_back(i);
          }
          else { // nothing to do
          }
        });
        std::for_each(visitedBackward.rbegin(), visitedBackward.rend(), [&visitedLetters, shiftedI, shiftedJ](auto i){
          if(i != shiftedI && i != shiftedJ) {
            visitedLetters.push_back(i);
          }
          else { // nothing to do
          }
        });
/*std::cout << i.first << ':' << j.first << ' ' << distance << ':' << visited.size() << ' ';
for(auto &i : mVisitedLetters[compactY][compactX]) {
  std::cout << static_cast<char>(64+i);
}
std::cout << '\n';*/
      }
    }
  }

  size_t calculateShortestPath() {
    size_t iterations = 0u;
    size_t result = std::numeric_limits<size_t>::max();
    size_t pointer = 0u;
    std::array<Iteration, cRecursionDepth> stack;
    auto &bottom = stack[0u];
    bottom.previous = cCompactStart;
    bottom.lengthSoFar = 0u;
    bottom.choices = mKeys;
    std::fill(bottom.visited.begin(), bottom.visited.end(), false);
    bottom.visited[cCompactStart] = true;
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
        next.choices = current.choices;
        next.visited = current.visited;
        next.visited[previous] = true;
        next.choices[previous] = false;
        for(auto &i : mVisitedLetters[prePrevious][previous]) {
          next.visited[i] = (next.visited[i] || next.choices[i]);
          next.choices[i] = false;
          if(i >= cCompactKeyStart) {
            char door = i - cCompactDoor2key;
            next.choices[door] = mDoors[door] & !next.visited[door];
          }
        }
        if(previous >= cCompactKeyStart) {
          char door = previous - cCompactDoor2key;
          next.choices[door] = mDoors[door];
        }
        else { // nothing to do
        }
        
      }
    }
std::cout << "iterations: " << iterations << '\n';
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
    if(what != cCorridor) {
      aVisited.push_back(what - cCompactOffset);
    }
    else { // nothing to do
    }
  }
};

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
