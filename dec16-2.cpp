#include <array>
#include <deque>
#include <vector>
#include <chrono>
#include <cctype>
#include <limits>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>

// This is not a solution. If I had the patience, I could write a general solution using two cases.
// Let the length of the non-repeated input be L and the position to be calculated in the repeated input P.
// During an FFT phase, a length of LCM(L, 4P) sums up to 0. So it is enough the calculat the actual sum for
// the last 10000L % LCM(L, 4P) digits. Even that calculation could perhaps use some tricks.
// For Ps where the LCM > 10000L, skipping the leading zeros and observing which sections fall into
// the bursts of 1s or -1s would let also smart calculations be done. Altogether, far less than
// the naive 10000L^2 calculations would suffice for one FFT turn.
//
// It turned out that the offset is given so that only the last few digits have to be summed without
// any modulo magic. As I wasn't smart enough to discover it myself, I won't use the trick here.

class Fft final {
private:
  static constexpr int    cZeroChar      = '0';
  static constexpr size_t cWaveletLength = 4u;
  static constexpr int            cWavelet[] = {0, 1, 0, -1};

  std::deque<int>                  mOriginal;
  std::array<std::vector<int>, 2u> mSignal;
  size_t                           mWhich;
  size_t                           mLength;

public:
  Fft(std::ifstream &aIn) {
    while(true) {
      int input = aIn.get();
      if(!isdigit(input)) {
        break;
      }
      else { // nothing to do
      }
      mOriginal.push_back(input - cZeroChar);
    }
    mLength = mOriginal.size();
std::cout << mLength << '\n';
    mSignal[0u] = std::vector<int>(mLength, 0);
    std:copy(mOriginal.begin(), mOriginal.end(), mSignal[0u].begin());
    mSignal[1u] = std::vector<int>(mLength, 0);
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
    auto begin = std::chrono::high_resolution_clock::now();
    fft.compute(100u);
    auto end = std::chrono::high_resolution_clock::now();
    auto timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
    std::cout << "duration: " << timeSpan.count() << '\n';
    fft.write(std::cout, 8u);
  }
  catch(std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
