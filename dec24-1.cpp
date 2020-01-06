#include <chrono>
#include <string>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <unordered_set>

class Population final {
private:
  static constexpr uint64_t cSize            = 5ull;
  static constexpr uint64_t cBitsPerPlace    = 2ull;
  static constexpr uint64_t cVerticalShift   = cSize * cBitsPerPlace;
  static constexpr uint64_t cBugShift        = 1ull;
  static constexpr uint64_t cLeftMask        = 0x0000ff3f6ff3f6ffull;
  static constexpr uint64_t cRightMask       = 0x0003f6ff3f6ff3f6ull;
  static constexpr uint64_t cBugMask         = 0x0001555555555555ull;
  static constexpr uint64_t cPopulationMask  = 0x0003ffffffffffffull;
                                            //   fedcba9876543210

  uint64_t                     mPopulation = 0ull;
  std::unordered_set<uint64_t> mPreviousLayouts;

public:
  Population(std::ifstream &aIn) {
    size_t readSoFar;
    for(readSoFar = 0u; readSoFar < cSize; ++readSoFar) {
      std::string line;
      std::getline(aIn, line);
      size_t readFromLine;
      for(readFromLine = 0u; readFromLine < cSize && readFromLine < line.length(); ++readFromLine) {
        mPopulation |= (line[readFromLine] == '#' ? 1ull << ((readSoFar * cSize + readFromLine) * cBitsPerPlace) : 0u);
      }
      if(!aIn.good() || readFromLine != cSize) {
        break;
      }
      else { // nothing to do
      }
    }
    if(readSoFar != cSize) {
      throw std::invalid_argument("Invalid initial popuilation");
    }
    else { // nothing to do
    }
    mPopulation &= cPopulationMask;
    mPreviousLayouts.insert(mPopulation);
  }

  uint64_t compute() {
    while(stepWithoutMatch()) {
    }
    uint64_t result = 0u;
    for(uint64_t i = 0u; i < cSize * cSize; ++i) {
      result += ((mPopulation & (1ull << (i * cBitsPerPlace))) > 0u ? 1ull << i : 0u);
    }
    return result;
  }

private:
  bool stepWithoutMatch() {
    uint64_t sum = mPopulation << cVerticalShift;
    sum += (mPopulation >> cVerticalShift);
    sum += (mPopulation & cLeftMask) << 2u;
    uint64_t right = (mPopulation & cRightMask) >> 2u;
    uint64_t upperBits = (sum >> cBugShift) & cBugMask;
    uint64_t lowerBits = sum & cBugMask;
    uint64_t already3 = upperBits & lowerBits;
    right &= ~already3;
    sum += right;
    upperBits = (sum >> cBugShift) & cBugMask;
    lowerBits = sum & cBugMask;
    uint64_t newPopulation = mPopulation & lowerBits & ~upperBits;
    mPopulation = (newPopulation | (~mPopulation & (lowerBits ^ upperBits))) & cPopulationMask;
    bool found = mPreviousLayouts.find(mPopulation) != mPreviousLayouts.end();
    mPreviousLayouts.insert(mPopulation);
    return !found; 
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
    Population population(in);
    auto begin = std::chrono::high_resolution_clock::now();
    uint64_t result = population.compute();
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
