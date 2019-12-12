#include <array>
#include <deque>
#include <limits>
#include <cctype>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>

// Strange... the moons attract each other, but the Jupiter has no effect on them (-:
class Moon final {
private:
  static size_t constexpr cDimensions = 3u;

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
  static size_t constexpr cCycles = 1000u;

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

  int getEnergyAfterAwhile() {
    for(size_t i = 0u; i < cCycles; ++i) {
      for(auto &one : mMoons) {
        for(auto &other : mMoons) {
          one.updateVelocity(other);
        }
      }
      for(auto &one : mMoons) {
        one.updatePosition();
      }
    }
    int energy = 0;
    for(auto &one : mMoons) {
      energy += one.getEnergy();
    }
    return energy;
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

    LunarSystem lunarSystem(in);
    
    std::cout << lunarSystem.getEnergyAfterAwhile() << '\n';
  }
  catch(std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
