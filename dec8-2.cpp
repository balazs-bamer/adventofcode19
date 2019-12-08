#include <limits>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>

char   const cColourBlack       = '0';
char   const cColourWhite       = '1';
char   const cColourTransparent = '2';
char   const cRenderBlack       = 'o';
char   const cRenderWhite       = '#';
char   const cRenderTransparent = ' ';

void flatten(char const * const aInputImage, char * const aFinalImage, size_t const aLayerSize) noexcept {
  std::transform(aInputImage, aInputImage + aLayerSize, aFinalImage, aFinalImage, [](char aInput, char aFinalCandidate) {
    char result;
    if(aFinalCandidate == cRenderTransparent) {
      if(aInput == cColourBlack) {
        result = cRenderBlack;
      }
      else if(aInput == cColourWhite) {
        result = cRenderWhite;
      }
      else {
        result = cRenderTransparent;
      }
    }
    else {
      result = aFinalCandidate;
    }
    return result;
  });
}

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
    char   inputImage[cLayerSize];
    char   finalImage[cLayerSize];
    size_t pixelCount = 0u;
    
    std::fill(finalImage, finalImage + cLayerSize, cRenderTransparent);
    std::fill(inputImage, inputImage + cLayerSize, cColourTransparent);
    while(true) {
      inputImage[pixelCount] = in.get();
      if(!in.good()) {
/* for checking trncated last layer        if(pixelCount != 0u) {
          throw std::invalid_argument("Invalid file");
        }
        else { // nothing to do
        }*/
        break;
      }
      else { // nothing to do
      }
      if(++pixelCount == cLayerSize) {
        flatten(inputImage, finalImage, cLayerSize);
        std::fill(inputImage, inputImage + cLayerSize, cColourTransparent);
        pixelCount = 0u;
      }
      else { // nothing to do
      }
    }
    flatten(inputImage, finalImage, cLayerSize);
    pixelCount = 0u;
    for(size_t row = 0u; row < cHeight; ++row) {
      for(size_t column = 0u; column < cWidth; ++column) {
        std::cout << finalImage[pixelCount];
        ++pixelCount;
      }
      std::cout << '\n';
    }
  }
  catch(std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
