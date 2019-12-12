#include <array>
#include <deque>
#include <limits>
#include <cctype>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>

class LunarSystem;

// Strange... the moons attract each other, but the Jupiter has no effect on them (-:
class Moon final {
friend class LunarSystem;
public:
  static size_t constexpr cDimensions = 3u;

private:
  std::array<int, cDimensions> mPosition;
  std::array<int, cDimensions> mVelocity;
  bool initialized = false;

public:
  Moon() noexcept = default;

  Moon(std::ifstream &aIn) {
    for(auto &i : mPosition) {
      aIn.ignore(3u);
      aIn >> i;
      aIn.ignore(1u);
    }
    std::fill(mVelocity.begin(), mVelocity.end(), 0);
    initialized = aIn.good();
    aIn.ignore(1u);
  }

  bool operator!() noexcept {
    return !initialized;
  }

  void updateVelocity(Moon const &aOther) noexcept {
    for(size_t i = 0u; i < cDimensions; ++i) {
      if(mPosition[i] > aOther.mPosition[i]) {
        --mVelocity[i];
      }
      else if(mPosition[i] < aOther.mPosition[i]) {
        ++mVelocity[i];
      }
      else { // nothing to do
      }
    }
  }

  void updatePosition() noexcept {
    std::transform(mPosition.begin(), mPosition.end(), mVelocity.begin(), mPosition.begin(), [](auto const aPos, auto const aVel) {
      return aPos + aVel;
    });
  }

  int getEnergy() noexcept {
    int kinetic = 0;
    int potential = 0;
    for(size_t i = 0u; i < cDimensions; ++i) {
      kinetic += abs(mVelocity[i]);
      potential += abs(mPosition[i]);
    }
    return kinetic * potential;
  }
};

class LunarSystem final {
private:
  std::deque<Moon> mMoons;

public:
  LunarSystem(std::ifstream &aIn) {
    while(true) {
      Moon moon(aIn);
      if(!moon) {
        break;
      }
      else {
        mMoons.push_back(moon);
      }
    }
  }

  LunarSystem &operator=(LunarSystem const &aOther) {
    mMoons = aOther.mMoons;
  }

  void step() noexcept {
    for(auto &one : mMoons) {
      for(auto &other : mMoons) {
        one.updateVelocity(other);
      }
    }
    for(auto &one : mMoons) {
      one.updatePosition();
    }
  }

  bool equalsAtDimension(size_t const aDimension, LunarSystem const &aOther) const noexcept {
    bool equals = true;
    for(size_t i = 0u; i < mMoons.size(); ++i) {
      if(mMoons[i].mPosition[aDimension] != aOther.mMoons[i].mPosition[aDimension] ||
         mMoons[i].mVelocity[aDimension] != aOther.mMoons[i].mVelocity[aDimension]) {
        equals = false;
      }
      else { // nothing to do
      }
    }
    return equals;
  } 
};

size_t lcm(size_t aOne, size_t aOther) noexcept {
  size_t product = aOne * aOther;
  while(aOther != 0u) {
    size_t tmp = aOther;
    aOther = aOne % aOther;
    aOne = tmp;
  }
  return product / aOne;
}

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

    LunarSystem original(in);
    size_t cycles = 1u;
    for(size_t i = 0u; i < Moon::cDimensions; ++i) {
      LunarSystem lunarSystem = original;
      size_t count = 0u;
      do {
        lunarSystem.step();
        ++count;
      } while(!lunarSystem.equalsAtDimension(i, original));
      cycles = lcm(cycles, count);
    }
    std::cout << cycles << '\n';
  }
  catch(std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
