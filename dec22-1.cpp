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

bool constexpr isPrime(int aNumber) noexcept {
  bool result = true;
  for(int i = 2; i < aNumber - 1; ++i) {
    if(aNumber % i == 0) {
      result = false;
    }
    else { // nothing to do
    }
  }
  return result;
}

template<int tSize>
class Deck final {
static_assert(tSize > 1 && isPrime(tSize));
private:
  std::array<int, tSize> mDeck;

public:
  Deck() noexcept {
    for(int i = 0; i < tSize; ++i) {
      mDeck[i] = i;
    }
  }

  void reverse() {
    std::reverse(mDeck.begin(), mDeck.end());
  }

  void cut(int const aCut) {
    int realCut = limit(aCut);
    std::rotate(mDeck.begin(), mDeck.begin() + realCut, mDeck.end());
  }

  void deal(int const aIncrement) {
    int realInc = limit(aIncrement);
    if(realInc > 1) {
      std::array<int, tSize> tmp;
      for(int i = 0; i < tSize; ++i) {
        tmp[(i * realInc) % tSize] = mDeck[i];
      }
      mDeck = tmp;   
    }
    else { // nothing to do
    }
  }

  int operator[](int const aIndex) const noexcept {
    return mDeck[limit(aIndex)];
  }

void print() const noexcept {
  for(int i : mDeck) {
    std::cout << i << ' ';
  }
  std::cout << '\n';
}

private:
  int limit(int const aValue) const noexcept {
    int real = aValue;
    while(real < 0) { // It won't get extreme negative values, so remains efficient.
      real += tSize;
    }
    return real % tSize;
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
  int parameter;
  
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
  static constexpr int cSpaceDeckSize   = 10007;
  static constexpr int cIndexInQuestion =  2019;

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

  int compute() const {
    Deck<cSpaceDeckSize> deck;
    for(auto &i : mSteps) {
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
    }
    int result;
    for(result = 0; result < cSpaceDeckSize; ++result) {
      if(deck[result] == cIndexInQuestion) {
        break;
      }
      else { // nothing to do
      }
    }
    return result;
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
    int result = shuffle.compute();
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
