#include <cmath>
#include <deque>
#include <cctype>
#include <limits>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>

// there exists an Time:O(nlogn) and Space:O(n) algorithm but I don't have time to code it

class Collector final {
private:
  std::deque<int> mHorizontals;
  std::deque<int> mVerticals;
  bool                mHorizontalStart;
  char                mLastDirection = 0;
    
public:
  void insert(char aDirection, int aLength) {
    if(aDirection == mLastDirection ||
       aDirection == 'L' && mLastDirection == 'R' ||
       aDirection == 'R' && mLastDirection == 'L' ||
       aDirection == 'U' && mLastDirection == 'D' ||
       aDirection == 'D' && mLastDirection == 'U' ||
       aLength == 0) {
      throw std::invalid_argument("Line not bent.");
    }
    if(mLastDirection == 0) {
      mHorizontalStart = (aDirection == 'L' || aDirection == 'R');
    }
    else { // nothing to do
    }
    if(aDirection == 'U') {
      mVerticals.push_back(aLength);
    }
    else if(aDirection == 'D') {
      mVerticals.push_back(-aLength);
    }
    else if(aDirection == 'R') {
      mHorizontals.push_back(aLength);
    }
    else if(aDirection == 'L') {
      mHorizontals.push_back(-aLength);
    }
    mLastDirection = aDirection;
  }

  int getNearestIntersectionDistance(Collector &aOther) const {
    int distance = std::numeric_limits<int>::max();
    int x = 0;
    int y = 0;
    int length = 0;
    auto horizontal = mHorizontals.begin();  
    auto vertical = mVerticals.begin();
    if(mHorizontalStart) {
      if(horizontal != mHorizontals.end()) {
        distance = std::min(distance, aOther.measure(length, x, y, *horizontal, true));
        x += *horizontal;
        length += abs(*horizontal);
        ++horizontal;
      }
      else { // nothing to do
      }
    }
    else { // nothing to do
    }
    for(;;) {
      if(vertical != mVerticals.end()) {
        distance = std::min(distance, aOther.measure(length, x, y, *vertical, false));
        y += *vertical;
        length += abs(*vertical);
        ++vertical;
      }
      else {
        break;
      }
      if(horizontal != mHorizontals.end()) {
        distance = std::min(distance, aOther.measure(length, x, y, *horizontal, true));
        x += *horizontal;
        length += abs(*horizontal);
        ++horizontal;
      }
      else {
        break;;
      }
    }
    return distance;
  }

private:
  int measure(int const aOtherLengthSoFar, int aStartX, int aStartY, int aDisplacement, bool const aHorizontal) const {
    int distance = std::numeric_limits<int>::max();
    int x = 0;
    int y = 0;
    int length = 0;
    auto horizontal = mHorizontals.begin();  
    auto vertical = mVerticals.begin();
    if(mHorizontalStart) {
      if(horizontal != mHorizontals.end()) {
        if(!aHorizontal) {
          distance = std::min(distance, testIntersection(length, x, y, *horizontal, aOtherLengthSoFar, aStartX, aStartY, aDisplacement));
        }
        else { // nothing to do
        }
        x += *horizontal;
        length += abs(*horizontal);
        ++horizontal;
      }
      else { // nothing to do
      }
    }
    else { // nothing to do
    }
    for(;;) {
      if(vertical != mVerticals.end()) {
        if(aHorizontal) {
          distance = std::min(distance, testIntersection(aOtherLengthSoFar, aStartX, aStartY, aDisplacement, length, x, y, *vertical));
        }
        else {
        }
        y += *vertical;
        length += abs(*vertical);
        ++vertical;
      }
      else {
        break;
      }
      if(horizontal != mHorizontals.end()) {
        if(!aHorizontal) {
          distance = std::min(distance, testIntersection(length, x, y, *horizontal, aOtherLengthSoFar, aStartX, aStartY, aDisplacement));
        }
        else {
        }
        x += *horizontal;
        length += abs(*horizontal);
        ++horizontal;
      }
      else {
        break;
      }
    }
    return distance;
  }

  int testIntersection(int const aHorLength, int const aHorStartX, int const aHorStartY, int const aHorDisplacement, int const aVertLength, int const aVertStartX, int const aVertStartY, int const aVertDisplacement) const noexcept {
    int distance = std::numeric_limits<int>::max();
    if(contains(aVertStartY, aVertDisplacement, aHorStartY) && contains(aHorStartX, aHorDisplacement, aVertStartX)) {
      distance = aHorLength + aVertLength + abs(aVertStartX - aHorStartX) + abs(aVertStartY - aHorStartY);
    }
    else { // nothing to do
    }
    return distance;
  }

  bool contains(int const aStart, int const aDisplacement, int const aTarget) const noexcept {
    return (aDisplacement > 0 && aStart < aTarget && aTarget < aStart + aDisplacement) ||
           (aDisplacement < 0 && aStart + aDisplacement < aTarget && aTarget < aStart);
  }
};

bool read(std::ifstream &aIn, Collector &aCollector) {
  bool result = true;
  if(aIn.peek() == '\n') {
    result = false;
    aIn.ignore(1u);
  }
  else {
    if(aIn.peek() == ',') {
      aIn.ignore(1u);
    }
    else { // nothing to do
    }
    char direction;
    int length;
    if(aIn.eof() || !aIn.good()) {
      result = false;
    }
    else {
      aIn >> direction >> length;
      aCollector.insert(direction, length);
    }
  }
  return result;
}

int main(int const argc, char **argv) {
  try {
    if(argc == 1) {
      throw std::invalid_argument("Need input filename.");
    }
    else { // nothing to do
    }
    std::ifstream in(argv[1]);
    Collector firstLine;
    Collector secondLine;
    while(read(in, firstLine)) {
    }
    while(read(in, secondLine)) {
    }
    std::cout << firstLine.getNearestIntersectionDistance(secondLine) << '\n';
  }
  catch(std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
