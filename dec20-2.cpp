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
#include <unordered_set>

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

class Node final {
private:
  static constexpr uint64_t    cShiftFrom  =  8u;
  static constexpr uint64_t    cShiftLevel = 16u;
  uint8_t                      mChosenPort;
  size_t                       mLevel;

public:
  Node(uint8_t aPort) noexcept : mChosenPort(aPort), mLevel(0u) {
  }

  uint8_t getChosenPort() const noexcept {
    return mChosenPort;
  } 

  size_t getLevel() const noexcept {
    return mLevel;
  }

  bool isTopLevel() const noexcept {
    return mLevel == 0u;
  }

  bool operator==(Node const aOther) const noexcept {
    return mChosenPort == aOther.mChosenPort && mLevel == aOther.mLevel;
  }

  Node &operator=(Node const &aOther) noexcept = default;

  void move(uint8_t const aPort, bool const aFromOutside) noexcept {
    mChosenPort = aPort;
    if(aFromOutside) {
      ++mLevel;
    }
    else {
      --mLevel;
    }
  }
}; 

template<>
struct std::hash<Node> {
  size_t operator()(Node const &aKey) const {
    return std::hash<uint8_t>{}(aKey.getChosenPort()) ^
          (std::hash<size_t>{}(aKey.getLevel()) << 1u);
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
  static uint16_t constexpr cItemShift        =   8u;
  static uint16_t constexpr cFirstPort        = 0u;
  static uint16_t constexpr cWarpCorridor     = static_cast<uint16_t>(cCorridor) << cItemShift;
  static uint16_t constexpr cWarpWall         = static_cast<uint16_t>(cWall)    << cItemShift;
  static uint16_t constexpr cWarpSpace        = static_cast<uint16_t>(cSpace)  << cItemShift;
  static char     constexpr cStartLabel[]     = "AA";
  static char     constexpr cTargetLabel[]    = "ZZ";
  static size_t   constexpr cMargin           =   4u;
  static size_t   constexpr cDepthLimit       = 99u;
  static size_t   constexpr cCostLimit        = 9999999u;

  std::deque<std::deque<uint8_t>>          mRawMap;
  std::vector<std::vector<uint16_t>>       mWarpMap;
  uint8_t                                  mNextPort = cFirstPort;
  std::deque<Coordinates>                  mPortCoordinates;
  std::unordered_map<uint8_t, std::string> mPort2label;
  std::vector<bool>                        mPortsOutside;    
  uint8_t                                  mPortStart; 
  uint8_t                                  mPortTarget;
  std::vector<std::vector<size_t>>         mPort2portCost;
  std::unordered_map<uint8_t, uint8_t>     mPort2itsPair;
  size_t                                   mStepEstimate;

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
            if(label == cStartLabel) {
              mPortStart = mNextPort;
            }
            else if(label == cTargetLabel) {
              mPortTarget = mNextPort;
            }
            else { // nothing to do
            }
            mPort2label[mNextPort] = label;
            mPortCoordinates.push_back(corridorLoc);
            mWarpMap[labelLoc.getY()][labelLoc.getX()] = mNextPort;
            mWarpMap[otherHalfLoc.getY()][otherHalfLoc.getX()] = mNextPort;
            ++mNextPort;
          }
          else { // nothing to do
          }
        }
        else {
          mWarpMap[y][x] = foundItem << cItemShift;
        }
      }
    }
  }
  
  void calculateShortestPathesBetweenPorts() {
    size_t size = mPortCoordinates.size();
    {
      std::vector<size_t> line(size, cInvalidResult);
      mPort2portCost = std::vector<std::vector<size_t>>(size, line);
    }
    mPortsOutside = std::vector<bool>(size, false);
    for(size_t source = 0u; source < size; ++source) {
      Coordinates sourceLoc = mPortCoordinates[source];
      for(size_t dest = 0u; dest < size; ++dest) {
        if(source == dest) {
          mPort2portCost[source][dest] = cInvalidResult;
        }
        else {
          mPort2portCost[source][dest] = calculateShortestPath(sourceLoc, mPortCoordinates[dest]);
          if(mPort2label[source] == mPort2label[dest]) {
            mPort2itsPair[source] = dest;
//std::cout << source << '-' << dest << '\n';
          }
          else { // nothing to do
          }
std::cout << source << '-' << mPort2label[source] << "   " << dest << '-' << mPort2label[dest] << "   " << mPort2portCost[source][dest] << '\n';
        }
      }
      if(sourceLoc.getX() <= cMargin || 
         sourceLoc.getY() <= cMargin || 
         mRawMap.front().size() - sourceLoc.getX() <= cMargin ||
         mRawMap.size() - sourceLoc.getY() <= cMargin) {
        mPortsOutside[source] = true;
      }
      else { // nothing to do
      }
//std::cout << source << ' ' << mPort2label[source] << " out: " << mPortsOutside[source] << ' ' << sourceLoc.getX() << ':' << sourceLoc.getY() <<'\n';
    }
  }

  void estimateStep() {
    size_t size = mPortCoordinates.size();
    mStepEstimate = cInvalidResult;
    for(size_t source = 0u; source < size; ++source) {
      Coordinates sourceLoc = mPortCoordinates[source];
      for(size_t dest = 0u; dest < size; ++dest) {
        if(source != dest && (mPortsOutside[source] ^ mPortsOutside[dest])) {
          mStepEstimate = std::min<size_t>(mStepEstimate, mPort2portCost[source][dest]);
        }
        else { // nothing to do
        }
      }
    }
    std::cout << "step estimate: " << mStepEstimate << '\n';
  }

  size_t calculateShortestPath() {
    size_t iterations = 0u;
    size_t result = cInvalidResult;
    std::unordered_map<Node, size_t> wholeCosts; // assume cInvalidResult for nodes not present here
    std::unordered_map<Node, size_t> realCosts; // assume cInvalidResult for nodes not present here
    std::multimap<size_t, Node> queue;
    Node start(mPortStart);
    wholeCosts[start] = mStepEstimate;
    realCosts[start] = 0u;
    queue.insert(std::pair<size_t, Node>(0u, start));
    while(!queue.empty()) {
      auto smallest = queue.begin();
      Node smallestNode = smallest->second;
      size_t smallestCost = realCosts[smallestNode];
//std::cout << smallestNode.getLevel() << ' ' << smallestCost << '\n';
      queue.erase(smallest);
      auto &nextPorts = mPort2portCost[smallestNode.getChosenPort()];
//std::cout << "line: " << mPort2label[smallestNode.getChosenPort()] << ' ' << smallestNode.getLevel() << '\n';
      for(uint8_t i = 0; i < nextPorts.size(); ++i) {        // i is the other port to be chosen on this level. It may lead to a lower or an upper level.
        if((smallestNode.isTopLevel() && mPortsOutside[i] && i != mPortTarget) ||   // on top level only target live on the outside
          (!smallestNode.isTopLevel() && mPortsOutside[i] && (i == mPortStart || i == mPortTarget))) { // on lower levels start and target are stuck
//std::cout << "cont: " << mPort2label[i] << ' ' << smallestNode.getLevel() << '\n';
          continue;
        }
        else { // nothing t odo
        }
        Node otherNode = smallestNode;
        uint8_t newPort = mPort2itsPair[i];
        otherNode.move(newPort, mPortsOutside[newPort]);
        auto other = realCosts.find(otherNode);
        size_t otherRealCost = (other == realCosts.end() ? cInvalidResult : other->second);
        size_t costIncrement = nextPorts[i];
        if(costIncrement == cInvalidResult) {
//std::cout << "inv: " << i << ' ' << newPort << '\n';
          continue;
        }
        else { // nothing t odo
        }
        size_t newRealCost = smallestCost + costIncrement + 1u;
//std::cout << "exam: " << mPort2label[i] << ' ' << smallestNode.getLevel() << ' ' << static_cast<int>(i) << '-' << static_cast<int>(mPortTarget) << ' ' << newRealCost << '-' << result <<'\n';
        if(i == mPortTarget) {
          if(newRealCost < result) {
            result = newRealCost;
  //std::cout << "perhaps " << (result - 1u) << " at " << iterations << std::endl;
          }
          else { // nothing to do
          }
          continue;
        }
        else {
//std::cout << "nrc: " << newRealCost << " orc: " << otherRealCost << '\n';
          if(newRealCost < otherRealCost && newRealCost < cCostLimit && otherNode.getLevel() < cDepthLimit) {
            if(other == realCosts.end()) {
              realCosts[otherNode] = newRealCost;
            }
            else {
              other->second = newRealCost;
            }
            size_t newWholeCost = newRealCost + mStepEstimate * otherNode.getLevel();
            wholeCosts[otherNode] = newWholeCost;
            queue.insert(std::pair<size_t, Node>(newWholeCost, otherNode));
  std::cout << smallestNode.getLevel() << '-'<<static_cast<uint16_t>(smallestNode.getChosenPort()) << '-' << mPort2label[smallestNode.getChosenPort()] << ' ' << " <[" << nextPorts[i] << "]> " << otherNode.getLevel() << '-'<<static_cast<uint16_t>(otherNode.getChosenPort()) << '-' << mPort2label[otherNode.getChosenPort()] <<  ' ' <<'\n';
          }
          else { // nothing to do
          }
        }
      }
      ++iterations;
    }
std::cout << "iter: " << iterations << '\n';
    return result - 1u;
  }

private:
  size_t calculateShortestPath(Coordinates const aStart, Coordinates const aTarget) {
    size_t iterations = 0u;
    size_t result = cInvalidResult;
    Coordinates lastNode;
    std::unordered_map<Coordinates, size_t> wholeCosts; // assume cInvalidResult for nodes not present here
    std::multimap<size_t, Coordinates> queue;
    wholeCosts[aStart] = 0u;
    queue.insert(std::pair<size_t, Coordinates>(0u, aStart));
    while(!queue.empty()) {
      auto smallest = queue.begin();
      size_t smallestCost = smallest->first;
      Coordinates smallestNode = smallest->second;
      queue.erase(smallest);
      if(smallestNode == aTarget) {
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
        if(found == cWarpSpace || found == cWarpWall) {
          continue;
        }
        else if(found != cWarpCorridor) {
          otherNode = mPortCoordinates[found];
          if(otherNode.isOrigin()) {
            continue;
          }
          else { // nothing to do
          }
        }
        else { // nothing to do
        }
        auto other = wholeCosts.find(otherNode);
        size_t otherCost = (other == wholeCosts.end() ? cInvalidResult : other->second);
        size_t newCost = smallestCost + 1u;
        if(newCost < otherCost) {
          if(other == wholeCosts.end()) {
            wholeCosts[otherNode] = newCost;
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
    labyrinth.calculateShortestPathesBetweenPorts();
    labyrinth.estimateStep();
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
