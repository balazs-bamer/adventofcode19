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
  std::set<Result> mSaved;
  std::map<Material, size_t> mLeftovers;
  size_t mOresLeft = 1000000000000u;
  static Material  sOre;

public:
  Components &operator+=(Result const &aComponent) {
    mComponents.insert(aComponent);
    return *this;
  }

  static void setOreName(Material const aOre) noexcept {
    sOre = aOre;
  }

  void save() {
    mSaved = mComponents;
  }

  void restore() {
    mComponents = mSaved;
  }

  bool empty() const noexcept {
    return mComponents.empty();
  }

  Result fetch() {
    auto head = mComponents.begin();
    Result result = *head;
    mComponents.erase(head);
    return result;
  }

  bool modify(Components const &aNewComponents, size_t const aResultQuantity, Result const &aToReplace) {
    auto found = mLeftovers.find(aToReplace.material);
    size_t leftover = (found == mLeftovers.end() ? 0u : found->second);
    size_t multiplier = ((aToReplace.quantity - leftover + aResultQuantity - 1u) / aResultQuantity);
    leftover += multiplier * aResultQuantity - aToReplace.quantity;
    mLeftovers[aToReplace.material] = leftover;
    if(multiplier > 0u) {
      for(auto &i : aNewComponents.mComponents) {
        std::set<Result>::iterator found = mComponents.find(i);
        if(found == mComponents.end()) {
          if(i.material == sOre) {
            if(multiplier * i.quantity > mOresLeft) {
              return false;
            }
            else {
              mOresLeft -= multiplier * i.quantity;
            }
          }
          else {
            mComponents.emplace(i.material, multiplier * i.quantity);
          }
        }
        else {
          const_cast<size_t&>(found->quantity) += multiplier * i.quantity;
        }
      }
    }
    else { // nothing to do
    }
    return true;
  }
};

Material Components::sOre;

class Reactor final {
private:
  Dictionary                        mMaterialNames;
  std::multimap<Result, Components> mPossibleReactions;
  Components                        mReaction;

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
      if(result.material == mMaterialNames.getFuel()) {
        mReaction = components;
      }
      else {
        mPossibleReactions.insert(std::pair<Result, Components>(result, components));
      }
      while(aIn.good() && (aIn.peek() == ' ' || aIn.peek() == '\n')) {
        aIn.ignore(1u);
      }
    }
  }

  size_t calculateFuel() {
    size_t result = 0u;
    Components::setOreName(mMaterialNames.getOre());
    mReaction.save();
    while(calculateOres()) {
      ++result;
      mReaction.restore();
    }
    return result;
  }

  bool calculateOres() {
    while(!mReaction.empty()) {
      Result component = mReaction.fetch();
      auto toModify = mPossibleReactions.find(component);
      if(toModify == mPossibleReactions.end()) {
        throw std::invalid_argument("Component without reaction.");
      }
      else { // nothing to do
      }
      if(!mReaction.modify(toModify->second, toModify->first.quantity, component)) {
        return false;
      }
    }
    return true;
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
    size_t result = reactor.calculateFuel();
    auto end = std::chrono::high_resolution_clock::now();
    auto timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
    std::cout << "duration: " << timeSpan.count() << '\n'; // 268.045
    std::cout << result << '\n';
  }
  catch(std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
