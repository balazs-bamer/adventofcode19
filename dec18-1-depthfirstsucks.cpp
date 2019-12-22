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

// Depth first search to make distance matrix generation faster, but it fails because of the clearance arond @

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
  static constexpr uint8_t cAllSeen = 15u;

  char        mWhat;           
  Coordinates mParent;            // origin means root
  size_t      mDistanceFromRoot = 0u;
  uint8_t     mDirSeen          = 0u;

public:
  Node(char aWhat, Coordinates &aParent, size_t const aDistanceFromRoot) noexcept : mWhat(aWhat), mParent(aParent), mDistanceFromRoot(aDistanceFromRoot) {
  }

  Node(Node const &aNode) = default;

  void seen(int const aDir) noexcept {
    mDirSeen |= 1u << aDir;
  }

  bool hasSeenAll() const noexcept {
    return mDirSeen == cAllSeen;
  }

  Coordinates getParentLocation() const noexcept {
    return mParent;
  }

  size_t getDistanceFromRoot() const noexcept {
    return mDistanceFromRoot;
  }

  char getWhat() const noexcept {
    return mWhat;
  } 
};

class Chain final {
private:
  std::list<Coordinates> mList;

public:
  Chain() = default;

  void prepend(Coordinates const &aHere) {
    mList.push_front(aHere);
  }
};

class Labyrinth final {
private:
  static char constexpr cStart          = '@';
  static char constexpr cCorridor       = '.';
  static char constexpr cFootprint      = '~';
  static char constexpr cWall           = '#';
  static char constexpr cNewLine        = '\n';
  static int  constexpr cDirCount       = 4;
  static int  constexpr cStraightLimit  = 2;
  static int  constexpr cDirSearchStart = 3;

  int                                   mStartX  = 0;
  int                                   mStartY  = 0;
  std::deque<std::deque<char>>          mMap;
  std::unordered_map<Coordinates, Node> mTree;
  std::unordered_map<char, Chain>       mChains;
  std::unordered_map<char, size_t>>     mDictionary;
  std::vector<std::vector<size_t>>      mDistances;

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
  for(auto &ch : line) {
std::cout << ch;
  }
std::cout << '\n';
}
std::cout << 'x' << mStartX << 'y' << mStartY << '\n';*/
  }

  void buildTree() {
    Coordinates parentLocation;
    Coordinates here(mStartX, mStartY);
    int dir = 0;
    size_t distanceFromRoot = 0u;
    while(true) {
      auto nodeHere = mTree.find(here);
      if(isNode(here)) {
        if(nodeHere == mTree.end()) {
          Node what = Node(mMap[here.getY()][here.getX()], parentLocation, distanceFromRoot);
          nodeHere = mTree.emplace(here, what).first;
        }
        else {
          distanceFromRoot = nodeHere->second.getDistanceFromRoot();            
        }
        parentLocation = here;
      }
      else { // nothing to do
      }
      if(nodeHere != mTree.end()) {
        if(nodeHere->second.hasSeenAll()) {
          break;
        }
        else { // nothing to do
        }
      }
      else { // nothing to do
      }
      std::optional<int> newDir = getNewDir(dir, here, nodeHere);
      if(newDir) {
        dir = *newDir;
        if(nodeHere != mTree.end()) {
          if(nodeHere->second.hasSeenAll()) {
            mMap[here.getY()][here.getX()] = cFootprint;
            parentLocation = nodeHere->second.getParentLocation();
            distanceFromRoot = nodeHere->second.getDistanceFromRoot();            
          }
          else { // nothing to do
          }
          ++distanceFromRoot;
        }
        else {
          distanceFromRoot += (mMap[here.getY()][here.getX()] == cFootprint ? 0u : 1u);
          mMap[here.getY()][here.getX()] = cFootprint;
        }
        here += dir;
      }
      else {
        break;
      }
    }
  }

  void buildLists() {
    for(auto &i : mTree) {
std::cout << (i.second.getWhat() == '~' || i.second.getWhat() == '.' ? "tree node " : "tree alph ");
std::cout << i.second.getWhat() << " x: " << i.first.getX() << " y: " << i.first.getY() << ' ' << i.second.getDistanceFromRoot() << '\n';
      if(i.second.getWhat() != cCorridor) {
        Chain chain;
        for(Corridor here = i.first; !here.isOrigin(); here = mTree[here].getParentLocation()) {
          chain.prepend(here);
        }
        mChains[i.second.getWhat()] = chain;
      }
      else { // nothing to do
      }
    }
  }

  void buildMatrix() {

    for(auto &i : mChains) {
      for(auto &j : mChains) {
      }
    }
  }

private:
  std::optional<int> getNewDir(int const aDir, Coordinates const &aHere, std::unordered_map<Coordinates, Node>::iterator const &aNodeHere) const {
    std::optional<int> result;
    for(int tried = 0; tried < cDirCount; ++tried) {
      int newDir = (aDir + cDirSearchStart + tried) % cDirCount;
      if(aNodeHere != mTree.end()) {
        aNodeHere->second.seen(newDir);
      }
      else { // nothing to do
      }
      Coordinates look = aHere + newDir;
      if(mMap[look.getY()][look.getX()] != cWall) {
        result = newDir;
        break;
      }
      else { // nothing to do
      }
    }
    return result;
  }

  bool isNode(Coordinates const aHere) const {
    char what = mMap[aHere.getY()][aHere.getX()];
    bool result = (what != cCorridor && what != cFootprint);
    int found = 0;
    for(int dir = 0; dir < cDirCount; ++dir) {
      Coordinates look = aHere + dir;
      found += (mMap[look.getY()][look.getX()] == cWall ? 0 : 1);
    }
    return result || found > cStraightLimit;
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
    labyrinth.buildTree();
    labyrinth.buildLists();
    auto end = std::chrono::high_resolution_clock::now();
    auto timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
    std::cout << "duration: " << timeSpan.count() << '\n';
//    std::cout << place << '\n';
  }
  catch(std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
