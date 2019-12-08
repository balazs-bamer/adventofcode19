#include <limits>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>

int main(int const argc, char **argv) {
  char const cColours = 3u;
  size_t const cWidth = 25u;
  size_t const cHeight = 6u;
  size_t const cLayerSize = cWidth * cHeight;

  try {
    if(argc == 1) {
      throw std::invalid_argument("Need input filename.");
    }
    else { // nothing to do
    }
    std::ifstream in(argv[1]);
    size_t product = 0;
    size_t fewestZeros = cLayerSize + 1u;
    size_t pixelCount = 0u;
    size_t colourCount[cColours];
    
    while(true) {
      if(pixelCount == 0u) {
        std::fill(colourCount, colourCount + cColours, 0u);
      }
      else { // nothing to do
      }
      char pixel;
      pixel = in.get() - '0';
      if(!in.good()) {
/* for checking trncated last layer        if(pixelCount != 0u) {
          throw std::invalid_argument("Invalid file");
        }
        else { // nothing to do
        }*/
        break;
      }
      ++colourCount[pixel];
      if(++pixelCount == cLayerSize) {
        if(colourCount[0] < fewestZeros) {
          fewestZeros = colourCount[0];
          product = colourCount[1] * colourCount[2];
        }
        else { // nothing to do
        }
        pixelCount = 0u;
      }
      else { // nothing to do
      }
    }
    std::cout << product << std::endl;
  }
  catch(std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
