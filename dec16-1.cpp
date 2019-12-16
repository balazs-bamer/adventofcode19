#include <array>
#include <deque>
#include <cctype>
#include <limits>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>

class Fft final {
private:
  static constexpr int    cZeroChar      = '0';
  static constexpr size_t cWaveletLength = 4u;
  static constexpr int            cWavelet[] = {0, 1, 0, -1};

  std::array<std::deque<int>, 2u> mSignal;
  size_t                          mWhich;
  size_t                          mLength;

public:
  Fft(std::ifstream &aIn) {
    mSignal[0u] = std::deque<int>();
    while(true) {
      int input = aIn.get();
      if(!isdigit(input)) {
        break;
      }
      else { // nothing to do
      }
      mSignal[0u].push_back(input - cZeroChar);
    }
    mLength = mSignal[0u].size();
    mSignal[1u] = std::deque<int>(mLength, 0);
  }

  void compute(size_t const aSteps) {
    for(mWhich = 0u; mWhich < aSteps; ++mWhich) {
      for(size_t pos = 1u; pos <= mLength; ++pos) {
        int sum = 0;
        size_t waveletIndex = (pos == 1u ? 1u : 0u);
        size_t repeatIndex  = (pos == 1u ? 0u : 1u);
        for(int digit : mSignal[mWhich % 2u]) {
          sum += digit * cWavelet[waveletIndex];
          repeatIndex = (repeatIndex + 1u) % pos;
          if(repeatIndex == 0u) {
            waveletIndex = (waveletIndex + 1u) % cWaveletLength;
          }
          else { // nothing to do
          }
        }
        mSignal[1u - mWhich % 2u][pos - 1u] = abs(sum) % 10;
      }
    }
  }

  void write(std::ostream &aOut, size_t const aLength) {
    std::for_each(mSignal[mWhich % 2u].begin(), mSignal[mWhich % 2u].begin() + aLength, [&aOut](int i){
      aOut << static_cast<char>(i + cZeroChar);
    });
    aOut << '\n';
  }
};

constexpr int Fft::cWavelet[];

int main(int const argc, char **argv) {
  size_t const cWidth             = 25u;
  size_t const cHeight            =  6u;
  size_t const cLayerSize = cWidth * cHeight;

  try {
    if(argc == 1) {
      throw std::invalid_argument("Need input filename.");
    }
    else { // nothing to do
    }
    std::ifstream in(argv[1]);
    Fft fft(in);
    fft.compute(100u);
    fft.write(std::cout, 8u);
  }
  catch(std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
