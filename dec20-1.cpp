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

// I assume there are no adjacent teleport labels.

class Coordinates final {
public:
  static int constexpr cDirNorth = 0;
  static int constexpr cDirEast  = 1;
  static int constexpr cDirSouth = 2;
  static int constexpr cDirWest  = 3;

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
  

class Labyrinth final {
private:
  static uint8_t  constexpr cNewLine          = '\n';
  static uint8_t  constexpr cCorridor         = '.';
  static uint8_t  constexpr cWall             = '#';
  static uint8_t  constexpr cSpace            = ' ';
  static size_t   constexpr cDirCount         =   4u;
  static size_t   constexpr cInvalidResult    = std::numeric_limits<size_t>::max();
  static uint16_t constexpr cWhichPortMask    = 128u;
  static uint16_t constexpr cFirstPort        = 256u;
  static uint16_t constexpr cPortIncrement    = 256u;
  static char     constexpr cStartLabel[]     = "AA";
  static char     constexpr cTargetLabel[]    = "ZZ";

  std::deque<std::deque<uint8_t>>           mRawMap;
  std::vector<std::vector<uint16_t>>        mWarpMap;
  uint16_t                                  mNextPort = cFirstPort;
  std::unordered_map<std::string, uint16_t> mLabel2port;
  std::unordered_map<uint16_t, Coordinates> mPort2coord;

public:
  Labyrinth(std::ifstream &aIn) {
    bool newLine = true;
    while(true) {
      if(newLine) {
        mRawMap.emplace_back(std::deque<uint8_t>());
        mRawMap.back().push_back(cSpace);
        newLine = false;
      }
      else { // nothing to do
      }
      uint8_t input = aIn.get();
      if(input == cNewLine) {
        mRawMap.back().push_back(cSpace);
        newLine = true;
      }
      else {
        mRawMap.back().push_back(input);
      }
      if(!aIn.good()) {
        break;
      }
      else { // nothing to do
      }
    }
    while(mRawMap.back().size() <= 2u) {
      mRawMap.pop_back();
    }
    std::deque<uint8_t> line(mRawMap.back().size(), cSpace);
    mRawMap.push_front(line);
    mRawMap.push_back(line);
  }

  void warp() {
    {
      std::vector<uint16_t> line(mRawMap.front().size(), cSpace);
      mWarpMap = std::vector<std::vector<uint16_t>>(mRawMap.size(), line);
    }
    for(size_t y = 0u; y < mRawMap.size(); ++y) {
      for(size_t x = 0u; x < mRawMap.front().size(); ++x) {
        uint8_t foundItem = mRawMap[y][x];
        if(std::isalpha(foundItem)) {
          Coordinates foundLoc(x, y);
          Coordinates otherLoc;
          uint8_t otherItem;
          for(size_t i = 0u; i < cDirCount; ++i) {
            otherLoc = foundLoc + i;
            otherItem = mRawMap[otherLoc.getY()][otherLoc.getX()];
            if(std::isalpha(otherItem)) {
              break;
            }
            else { // nothing to do
            }
          }
          Coordinates labelLoc;
          Coordinates corridorLoc;
          std::string label;
          if(foundLoc.getX() < otherLoc.getX() || foundLoc.getY() < otherLoc.getY()) {
            label += foundItem;
            label += otherItem;
          }
          else {
            label += otherItem;
            label += foundItem;
          }
          Coordinates otherHalfLoc;
          if(foundLoc.getY() == otherLoc.getY()) {
            if(mRawMap[foundLoc.getY()][foundLoc.getX() - 1] == cCorridor) {
              corridorLoc = foundLoc + Coordinates::cDirWest;
              labelLoc = foundLoc;
              otherHalfLoc = otherLoc;
            }
            else {
              corridorLoc = otherLoc + Coordinates::cDirEast;
              labelLoc = otherLoc;
              otherHalfLoc = foundLoc;
            }
          }
          else {
            if(mRawMap[foundLoc.getY() - 1][foundLoc.getX()] == cCorridor) {
              corridorLoc = foundLoc + Coordinates::cDirNorth;
              labelLoc = foundLoc;
              otherHalfLoc = otherLoc;
            }
            else {
              corridorLoc = otherLoc + Coordinates::cDirSouth;
              labelLoc = otherLoc;
              otherHalfLoc = foundLoc;
            }
          }
          if(mWarpMap[otherHalfLoc.getY()][otherHalfLoc.getX()] == cSpace && mWarpMap[labelLoc.getY()][labelLoc.getX()] == cSpace) {
            auto found = mLabel2port.find(label);
            if(found == mLabel2port.end()) {
              mLabel2port[label] = mNextPort;
              mPort2coord[mNextPort | cWhichPortMask] = corridorLoc;
              mWarpMap[labelLoc.getY()][labelLoc.getX()] = mNextPort;
              mWarpMap[otherHalfLoc.getY()][otherHalfLoc.getX()] = mNextPort;
              mNextPort += cPortIncrement;
            }
            else {
              uint16_t pairPort = found->second;
              auto found = mPort2coord.find(pairPort);
              if(found == mPort2coord.end()) {
                mPort2coord[pairPort] = corridorLoc;
                mWarpMap[labelLoc.getY()][labelLoc.getX()] = (pairPort | cWhichPortMask);
                mWarpMap[otherHalfLoc.getY()][otherHalfLoc.getX()] = (pairPort | cWhichPortMask);
              }
              else { // nothing to do
              }
            }
          }
          else { // nothing to do
          }
        }
        else {
          mWarpMap[y][x] = foundItem;
        }
      }
    }
  }

  size_t calculateShortestPath() {
    Coordinates start = mPort2coord[mLabel2port[cStartLabel] | cWhichPortMask];
    Coordinates target = mPort2coord[mLabel2port[cTargetLabel] | cWhichPortMask];
    size_t iterations = 0u;
    size_t result = cInvalidResult;
    Coordinates lastNode;
    std::unordered_map<Coordinates, size_t> costs; // assume cInvalidResult for nodes not present here
    std::multimap<size_t, Coordinates> queue;
    costs[start] = 0u;
    queue.insert(std::pair<size_t, Coordinates>(0u, start));
    while(!queue.empty()) {
      auto smallest = queue.begin();
      size_t smallestCost = smallest->first;
      Coordinates smallestNode = smallest->second;
      queue.erase(smallest);
      if(smallestNode == target) {
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
        Coordinates otherNode = smallestCoord + i;
        uint16_t found = mWarpMap[otherNode.getY()][otherNode.getX()];
        if(found == cSpace || found == cWall) {
          continue;
        }
        else if(found != cCorridor) {
          otherNode = mPort2coord[found];
          if(otherNode.isOrigin()) {
            continue;
          }
          else { // nothing to do
          }
        }
        else { // nothing to do
        }
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
          queue.insert(std::pair<size_t, Coordinates>(newCost, otherNode));
        }
        else { // nothing to do
        }
      }
      ++iterations;
    }
std::cout << "iter: " << iterations << '\n';
std::cout << '\n';
    return result;
  }
};
  
size_t  constexpr Labyrinth::cInvalidResult;
uint8_t constexpr Labyrinth::cSpace;
char    constexpr Labyrinth::cStartLabel[];
char    constexpr Labyrinth::cTargetLabel[];

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
    labyrinth.warp();
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
