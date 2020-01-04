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
  InfInt mFinalPlaceInQuestion;
  InfInt mIndependentFactor;
  InfInt mDependentFactor;

public:
  Deck(int64_t const aSize, int64_t const aPlaceInQuestion) noexcept 
  : mSize(aSize)
  , mPlaceInQuestion(aPlaceInQuestion)
  , mFinalPlaceInQuestion(aPlaceInQuestion)
  , mIndependentFactor(0u)
  , mDependentFactor(1u) {
  }

  void reverse() {
    mPlaceInQuestion = mSize - 1 - mPlaceInQuestion;
    mIndependentFactor = mSize - 1 - mIndependentFactor;
    mDependentFactor = -mDependentFactor;
  }

  void cut(int64_t const aCut) {
    InfInt limited = limit(aCut);
    mPlaceInQuestion = (mPlaceInQuestion + limited) % mSize;
    mIndependentFactor += limited;
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
    InfInt limited = limit(tPrev);
    mPlaceInQuestion = (mPlaceInQuestion * limited) % mSize;
    mIndependentFactor *= limited;
    mDependentFactor *= limited;
  }

  InfInt operator*() const noexcept {
    return mPlaceInQuestion;
  }

  InfInt operator!() const noexcept {
    return (mIndependentFactor + mDependentFactor * mFinalPlaceInQuestion) % mSize;
  }

  InfInt getResult(int64_t const aShuffleCount) const noexcept {
    InfInt veryBig = power(mDependentFactor, aShuffleCount);
    InfInt tmp = mFinalPlaceInQuestion * veryBig;
    tmp += mIndependentFactor * (veryBig - 1) / (mDependentFactor - 1);
    return tmp % mSize;
  }

private:
  InfInt limit(InfInt const &aValue) const noexcept {
    InfInt real = aValue;
    while(real < 0) { // It won't get extreme negative values, so remains efficient.
      real += mSize;
    }
    return real % mSize;
  }

  InfInt power(InfInt const &aBase, InfInt const &aExponent) const noexcept {
    InfInt power2 = aBase;
    InfInt work = aExponent;
    InfInt result = 1;
    while(work > 0) {
auto begin = std::chrono::high_resolution_clock::now();
      if(work % 2 > 0) {
        result *= power2;
      }
      else { // nothing to do
      }
      work /= 2;
      power2 *= power2;
auto end = std::chrono::high_resolution_clock::now();
auto timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
std::cout << work.numberOfDigits() << ' ' << power2.numberOfDigits() << ' ' << timeSpan.count() << '\n';
    }
    return result;
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
  static constexpr int64_t cSpaceDeckSize   = 10007; //119315717514047;
  static constexpr int64_t cIndexInQuestion =            2020;
  static constexpr int64_t cShuffleCount    = 101741582076661;

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
std::cout << *deck << '\n';
std::cout << !deck << '\n';
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
    });
std::cout << *deck << '\n';
std::cout << !deck << '\n';
    return deck.getResult(cShuffleCount);
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
