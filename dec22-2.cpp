#include "infint.h"
#include <list>
#include <array>
#include <chrono>
#include <string>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>

class Deck final {
private:
  InfInt mSize;
  InfInt mPlaceInQuestion;

public:
  Deck(int64_t const aSize, int64_t const aPlaceInQuestion) noexcept : mSize(aSize), mPlaceInQuestion(aPlaceInQuestion) {
  }

  void reverse() {
    mPlaceInQuestion = mSize - 1 - mPlaceInQuestion;
  }

  void cut(int64_t const aCut) {
    mPlaceInQuestion = (mPlaceInQuestion + limit(aCut)) % mSize;
  }

  void deal(int64_t const aIncrement) {
    InfInt rNow = limit(aIncrement);
    InfInt rPrev = mSize;
    InfInt tNow = 1, tPrev = 0;
    while(rNow != 0) {
      InfInt q = rPrev / rNow;
      InfInt tmp = rPrev - q * rNow;
      rPrev = rNow; rNow = tmp;
      tmp = tPrev - q * tNow;
      tPrev = tNow; tNow = tmp;
    }
    mPlaceInQuestion = (mPlaceInQuestion * limit(tPrev)) % mSize;
  }

  InfInt operator*() const noexcept {
    return mPlaceInQuestion;
  }

private:
  InfInt limit(InfInt const aValue) const noexcept {
    InfInt real = aValue;
    while(real < 0) { // It won't get extreme negative values, so remains efficient.
      real += mSize;
    }
    return real % mSize;
  }
};

struct Step final {
  static constexpr char cPrefixReverse[] = "deal into new stack";
  static constexpr char cPrefixCut[]     = "cut ";
  static constexpr char cPrefixDeal[]    = "deal with increment ";
 
  enum class Sort : uint8_t {
    cReverse = 0u,
    cCut     = 1u,
    cDeal    = 2u
  } sort;
  int64_t parameter;
  
  Step(std::string const aLine) {
    if(aLine.find(cPrefixReverse) == 0u) {
      sort = Sort::cReverse;
    }
    else if(aLine.find(cPrefixCut) == 0u) {
      sort = Sort::cCut;
      parameter = std::stoi(aLine.substr(strlen(cPrefixCut)));
    }
    else if(aLine.find(cPrefixDeal) == 0u) {
      sort = Sort::cDeal;
      parameter = std::stoi(aLine.substr(strlen(cPrefixDeal)));
    }
    else {
      throw std::invalid_argument("Invalid argument for Step.");
    }
  }

  void print() const {
    if(sort == Sort::cReverse) {
      std::cout << cPrefixReverse << '\n';
    }
    else if(sort == Sort::cCut) {
      std::cout << cPrefixCut << parameter << '\n';
    }
    else
    if(sort == Sort::cDeal) {
      std::cout << cPrefixDeal << parameter << '\n';
    }
    else { // nothing to do
    }
  }
};

constexpr char Step::cPrefixReverse[];
constexpr char Step::cPrefixCut[];
constexpr char Step::cPrefixDeal[];

class Shuffle final {
private:
  static constexpr int64_t cSpaceDeckSize   = 119315717514047;
  static constexpr int64_t cIndexInQuestion =            2020;

  std::list<Step> mSteps;

public:
  Shuffle(std::ifstream &aIn) {
    while(true) {
      std::string line;
      std::getline(aIn, line);
      if(!aIn.good()) {
        break;
      }
      else { // nothing to do
      }
      mSteps.emplace_back(line);
    }
  }

  InfInt compute() const {
    Deck deck(cSpaceDeckSize, cIndexInQuestion);
std::cout << *deck << "\n\n";
    std::for_each(mSteps.rbegin(), mSteps.rend(), [&deck](auto i){
      if(i.sort == Step::Sort::cReverse) {
        deck.reverse();
      }
      else if(i.sort == Step::Sort::cCut) {
        deck.cut(i.parameter);
      }
      else
      if(i.sort == Step::Sort::cDeal) {
        deck.deal(i.parameter);
      }
      else { // nothing to do
      }
i.print();
std::cout << *deck <<  "\n\n";
    });
    return *deck;
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
    Shuffle shuffle(in);
    auto begin = std::chrono::high_resolution_clock::now();
    InfInt result = shuffle.compute();
    auto end = std::chrono::high_resolution_clock::now();
    auto timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
    std::cout << "duration: " << timeSpan.count() << '\n';
    std::cout << result << '\n';
  }
  catch(std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
