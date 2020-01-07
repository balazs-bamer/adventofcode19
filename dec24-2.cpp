#include <list>
#include <array>
#include <chrono>
#include <string>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <algorithm>

class Population final {
private: 
  static constexpr size_t   cMinutes          = 5u; // 200u;
  static constexpr uint64_t cInitNeighbour    =   0ull;
  static constexpr uint64_t cSize             =   5ull;
  static constexpr uint64_t cBitsPerPlace     =   2ull;
  static constexpr uint64_t cVerticalShift    = cSize * cBitsPerPlace;
  static constexpr uint64_t cBugShift         =   1ull;
  static constexpr uint64_t cCrunchHorShift   =   5ull;
  static constexpr uint64_t cCrunchVertShift  =  29ull;
  static constexpr uint64_t cSouth2northShift =  40ull;
  static constexpr uint64_t cEast2westShift   =   8ull;
  static constexpr uint64_t cCenterNorthShift =  15ull;
  static constexpr uint64_t cCenterWestShift  =  23ull;
  static constexpr uint64_t cCenterEastShift  =  27ull;
  static constexpr uint64_t cCenterSouthShift =  35ull;
  static constexpr uint64_t cLeftMask         = 0x0000ff3f6ff3f6ffull;
  static constexpr uint64_t cRightMask        = 0x0003f6ff3f6ff3f6ull;
  static constexpr uint64_t cBugMask          = 0x0001555555555555ull;
  static constexpr uint64_t cPopulationMask   = 0x0003ffffffffffffull;
  static constexpr uint64_t cCenterNorthMask  = 0x0000000000004000ull;
  static constexpr uint64_t cCenterWestMask   = 0x0000000000400000ull;
  static constexpr uint64_t cCenterMask       = 0x0000000001000000ull;
  static constexpr uint64_t cCenterEastMask   = 0x0000000004000000ull;
  static constexpr uint64_t cCenterSouthMask  = 0x0000000400000000ull;
  static constexpr uint64_t cSideNorthMask    = 0x0000000000000155ull;
  static constexpr uint64_t cSideWestMask     = 0x0000010040100401ull;
  static constexpr uint64_t cSideEastMask     = 0x0001004010040100ull;
  static constexpr uint64_t cSideSouthMask    = 0x0001550000000000ull;
  static constexpr uint64_t cCrunchHorMask    = 0x000000000000001full;
  static constexpr uint64_t cCrunchVertMask   = 0x0000000000100c03ull;
  static constexpr size_t   cHorLookupWidth   = 5u;
  static constexpr size_t   cVertLookupSize   = 1ull << 21u;
                                            //   fedcba9876543210
  static constexpr uint64_t cHorLookup[1 << cHorLookupWidth]   = { 0u, 1u, 1u, 2u, 1u, 2u, 2u, 3u,
                                                                   1u, 2u, 2u, 3u, 2u, 3u, 3u, 3u,
                                                                   1u, 2u, 2u, 3u, 2u, 3u, 3u, 3u,
                                                                   2u, 3u, 3u, 3u, 3u, 3u, 3u, 3u };
  std::array<uint8_t, cVertLookupSize> mVertLookup;
  uint64_t                             mInitialPopulation;

public:
  Population(std::ifstream &aIn) {
    mInitialPopulation = 0ull;
    size_t readSoFar;
    for(readSoFar = 0u; readSoFar < cSize; ++readSoFar) {
      std::string line;
      std::getline(aIn, line);
      size_t readFromLine;
      for(readFromLine = 0u; readFromLine < cSize && readFromLine < line.length(); ++readFromLine) {
        mInitialPopulation |= (line[readFromLine] == '#' ? 1ull << ((readSoFar * cSize + readFromLine) * cBitsPerPlace) : 0u);
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
    mInitialPopulation &= cPopulationMask;
    std::fill(mVertLookup.begin(), mVertLookup.end(), 0u);
    uint64_t const bitConverter[] = { 0u, 1u, 10u, 11u, 20u };
    for(uint64_t i = 0u; i < cCrunchHorMask; ++i) {
      size_t vertIndex = 0u;
      for(size_t j = 0; j < cHorLookupWidth; ++j) {
        vertIndex += ((i & (1u << j)) > 0u ? 1u << bitConverter[j] : 0u);
      }
      mVertLookup[vertIndex] = cHorLookup[i];
    }
  }

  uint64_t compute() {
    std::array<std::list<uint64_t>, 2u> levels;
    size_t current = 0u;
    levels[current].push_back(mInitialPopulation);
    for(size_t i = 0; i < cMinutes; ++i) {
std::cout << "------------------ MINUTE: " << i << '\n';
      levels[1u - current].clear();
      levels[current].push_front(cInitNeighbour);
      levels[current].push_back(cInitNeighbour);
      for(auto level = levels[current].begin(); level != levels[current].end(); ++level) {
        uint64_t sum = getOwnSum(*level);
std::cout << "this: " << std::hex << std::setw(16) << *level;
std::cout << std::hex << std::setw(16) << sum << '\n';
        uint64_t correctionSum = 0u;
        if(level != levels[current].begin()) {
          auto previous = level;
          --previous;
std::cout << "prev: " << std::hex << std::setw(16) << *previous;
std::cout << std::hex << std::setw(16) << getOuterSum(*previous) << '\n';
          correctionSum |= getOuterSum(*previous);
        }
        else { // nothing to do
        }
        auto next = level;
        ++next;
        if(next != levels[current].end()) {
          correctionSum |= getInnerSum(*next);
std::cout << "next: " << std::hex << std::setw(16) << *next;
std::cout << std::hex << std::setw(16) << getInnerSum(*next) << '\n';
        }
        else { // nothing to do
        }
        levels[1u - current].push_back(merge(*level, sum, correctionSum));
std::cout << "res : " << std::hex << std::setw(16) << *level << '\n';
      }
      current = 1u - current;
    }
    uint64_t result = 0u;
    for(auto i : levels[current]) {
      result += countBits(i);
    }
    return result;
  }

private:
  uint64_t getOwnSum(uint64_t const aPopulation) const noexcept {
    uint64_t population = aPopulation & ~cCenterMask;
    uint64_t sum = population << cVerticalShift;
    sum += (population >> cVerticalShift);
    sum += (population & cLeftMask) << 2u;
    uint64_t right = (population & cRightMask) >> 2u;
    uint64_t upperBits = (sum >> cBugShift) & cBugMask;
    uint64_t lowerBits = sum & cBugMask;
    uint64_t already3 = upperBits & lowerBits;
    right &= ~already3;
    return sum + right;
  }

  // Four big tiles (from arg) influence each normal on the perimeter
  uint64_t getOuterSum(uint64_t const aPopulation) const noexcept {
    uint64_t sum  = ((aPopulation >> cCenterNorthShift) & 1u) * cSideNorthMask;
             sum += ((aPopulation >> cCenterWestShift) & 1u)  * cSideWestMask;
             sum += ((aPopulation >> cCenterEastShift) & 1u)  * cSideEastMask;
             sum += ((aPopulation >> cCenterSouthShift) & 1u) * cSideSouthMask;
    return sum;
  }

  // The small tiles on the inner level perimeter (from arg) influence the four normal ones around the center
  uint64_t getInnerSum(uint64_t const aPopulation) const noexcept {
    uint64_t north =  aPopulation & cSideNorthMask;
    uint64_t south = (aPopulation & cSideSouthMask) >> cSouth2northShift;
    uint64_t west  =  aPopulation & cSideWestMask;
    uint64_t east  = (aPopulation & cSideEastMask) >> cEast2westShift;
    uint64_t sum  = cHorLookup[(north | north >> cCrunchHorShift) & cCrunchHorMask] * cCenterNorthMask;
             sum |= cHorLookup[(south | south >> cCrunchHorShift) & cCrunchHorMask] * cCenterSouthMask;
             sum |= mVertLookup[(west | west >> cCrunchVertShift) & cCrunchVertMask] * cCenterWestMask;
             sum |= mVertLookup[(east | east >> cCrunchVertShift) & cCrunchVertMask] * cCenterWestMask;
    return sum;
  }

  uint64_t merge(uint64_t const aPopulation, uint64_t const aSumA, uint64_t const aSumB) const noexcept {
    uint64_t lowerBitsA = aSumA & cBugMask;
    uint64_t upperBitsA = (aSumA >> cBugShift) & cBugMask;
    uint64_t lowerBitsB = aSumB & cBugMask;
    uint64_t upperBitsB = (aSumB >> cBugShift) & cBugMask;
    uint64_t nonzeroA = lowerBitsA | upperBitsA;
    uint64_t nonzeroB = lowerBitsB | upperBitsB;
    uint64_t lowerBits = (upperBitsA & nonzeroB) | (upperBitsB & nonzeroA) | (lowerBitsA & !nonzeroB) | (lowerBitsB & !nonzeroA);
    uint64_t upperBits = upperBitsA | upperBitsB | (lowerBitsA & lowerBitsB);
    uint64_t newPopulation = aPopulation & lowerBits & ~upperBits;
    newPopulation |= (~aPopulation & (lowerBits ^ upperBits)) & cPopulationMask;
    return newPopulation & ~cCenterMask;
  }

  uint64_t countBits(uint64_t const aValue) const noexcept {
    uint64_t bitsPerByte[256];
    bitsPerByte[0] = 0u;
    for(size_t i = 0; i < 256; ++i) {
      bitsPerByte[i] = (i & 1u) + bitsPerByte[i / 2u];
    }
    uint64_t result = 0u;
    for(size_t i = 0; i < sizeof(uint64_t); ++i) {
      result += bitsPerByte[(aValue >> (i * 8u)) & 255];
    }
    return result;
  }
};
  
constexpr uint64_t Population::cHorLookup[];

int main(int const argc, char **argv) {
  try {
    if(argc == 1) {
      throw std::invalid_argument("Need input filename.");
    }
    else { // nothing to do
    }
std::cout << argv[1] << std::endl;
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
