#include <set>
#include <map>
#include <list>
#include <chrono>
#include <limits>
#include <cctype>
#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>

typedef size_t Material;

class Dictionary final {
private:
  static constexpr char           cOre[]  = "ORE";
  static constexpr char           cFuel[] = "FUEL";
  Material                        mOre;
  Material                        mFuel;
  std::map<std::string, Material> mDictionary;

public:
  Material readAndLookup(std::ifstream &aIn) {
    std::string name;
    while(aIn.good() && isalpha(aIn.peek())) {
      name += aIn.get();
    }
    auto found = mDictionary.find(name);
    Material result;
    if(found == mDictionary.end()) {
      result = mDictionary.size();
      mDictionary[name] = result;
      if(!name.compare(cOre)) {
        mOre = result;
      }
      else { // nothing to do
      }
      if(!name.compare(cFuel)) {
        mFuel = result;
      }
      else { // nothing to do
      }
    }
    else {
      result = found->second;
    }
    return result;
  }

  Material getOre() const noexcept {
    return mOre;
  }

  Material getFuel() const noexcept {
    return mFuel;
  }
};

constexpr char Dictionary::cOre[];
constexpr char Dictionary::cFuel[];

struct Result final {
public:
  Material material;
  size_t   quantity;

  Result(Material const aMaterial, size_t const aQuantity) : material(aMaterial), quantity(aQuantity) {
  }

  Result(std::ifstream &aIn, Dictionary &aDictionary) {
    aIn >> quantity;
    aIn.ignore(1u);
    material = aDictionary.readAndLookup(aIn);
  }

  bool operator<(Result const &aOther) const noexcept {
    return material < aOther.material;
  }

  bool operator==(Material const aMaterial) const noexcept {
    return material == aMaterial;
  }

  bool operator==(Result const &aOther) const noexcept {
    return material == aOther.material;
  }
};

class Components final {
private:
  std::set<Result> mComponents;
  std::map<Material, size_t> mLeftovers;
  size_t mOresLeft = 0u;
  static Material  sOre;

public:
  Components &operator+=(Result const &aComponent) {
    mComponents.insert(aComponent);
    return *this;
  }

  static void setOreName(Material const aOre) noexcept {
    sOre = aOre;
  }

  bool empty() const noexcept {
    return mComponents.empty();
  }

  size_t extractOres() {
    size_t result = 0u;
    for(auto i = mComponents.begin(); i != mComponents.end();) {
      if(i->material == sOre) {
        result += i->quantity;
        i = mComponents.erase(i);
      }
      else {
        ++i;
      }
    }
    return result;
  }

  Result fetch() {
    auto head = mComponents.begin();
    Result result = *head;
    mComponents.erase(head);
    return result;
  }

  Components branch(Components const &aNewComponents, size_t const aResultQuantity, Result const &aToReplace, size_t aOresSoFar) {
    Components components = *this;
    components.modify(aNewComponents, aResultQuantity, aToReplace);
    if(aOresSoFar > 0u) {
      components.mComponents.insert(Result(sOre, aOresSoFar));
    }
    else { // nothing to do
    }
    return components;
  }

  void modify(Components const &aNewComponents, size_t const aResultQuantity, Result const &aToReplace) {
    auto found = mLeftovers.find(aToReplace.material);
    size_t leftover = (found == mLeftovers.end() ? 0u : found->second);
    size_t multiplier = ((aToReplace.quantity - leftover + aResultQuantity - 1u) / aResultQuantity);
    leftover += multiplier * aResultQuantity - aToReplace.quantity;
    mLeftovers[aToReplace.material] = leftover;
    if(multiplier > 0u) {
      for(auto &i : aNewComponents.mComponents) {
        std::set<Result>::iterator found = mComponents.find(i);
        if(found == mComponents.end()) {
          mComponents.emplace(i.material, multiplier * i.quantity);
        }
        else {
          const_cast<size_t&>(found->quantity) += multiplier * i.quantity;
        }
      }
    }
    else { // nothing to do
    }
  }
};

Material Components::sOre;

class Reactor final {
private:
  Dictionary                   mMaterialNames;
  std::multimap<Result, Components> mPossibleReactions;
  std::list<Components>        mReactionsInProgress;

public:
  Reactor(std::ifstream &aIn) {
    while(aIn.good()) {
      Components components;
      do {
        Result result(aIn, mMaterialNames);
        components += result;  
        if(aIn.peek() == ',') {
          aIn.ignore(1u);
        }
        else { // nothing to do
        }
        aIn.ignore(1u);
      } while(aIn.peek() != '=');
      aIn.ignore(3u);
      Result result(aIn, mMaterialNames);
      mPossibleReactions.insert(std::pair<Result, Components>(result, components));
      while(aIn.good() && (aIn.peek() == ' ' || aIn.peek() == '\n')) {
        aIn.ignore(1u);
      }
    }
  }

  size_t findCheapest() {
    size_t result = std::numeric_limits<size_t>::max();
    gatherTargets();
    Components::setOreName(mMaterialNames.getOre());
    while(!mReactionsInProgress.empty()) {
      Components components = mReactionsInProgress.front();
      mReactionsInProgress.pop_front();
      result = std::min(result, calculateOres(components));
    }
    return result;
  }

private:
  void gatherTargets() {
    for(auto &i : mPossibleReactions) {
      if(i.first == mMaterialNames.getFuel()) {
        mReactionsInProgress.push_back(i.second);
      }
      else { // nothing to do
      }
    }
  }

  size_t calculateOres(Components &aComponents) {
    size_t ores = aComponents.extractOres();
    while(!aComponents.empty()) {
      Result component = aComponents.fetch();
      auto toModify = mPossibleReactions.find(component);
      if(toModify == mPossibleReactions.end()) {
        throw std::invalid_argument("Component without reaction.");
      }
      else { // nothing to do
      }
      auto toBranch = toModify;
      for(++toBranch; toBranch != mPossibleReactions.end(); ++toBranch) {
        if(toBranch->first.material == component.material) {
          mReactionsInProgress.push_back(aComponents.branch(toBranch->second, toBranch->first.quantity, component, ores));
        }
        else { // nothing to do
        }
      }
      aComponents.modify(toModify->second, toModify->first.quantity, component);
      ores += aComponents.extractOres();
    }
    return ores;
  }
};

int main(int const argc, char **argv) {
  size_t const cChainLength = 5u;
  int const cInitialInput = 0;

  try {
    if(argc == 1) {
      throw std::invalid_argument("Need input filename.");
    }
    else { // nothing to do
    }
    std::ifstream in(argv[1]);

    Reactor reactor(in);
    auto begin = std::chrono::high_resolution_clock::now();
    size_t result = reactor.findCheapest();
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
