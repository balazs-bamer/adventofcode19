#include <deque>
#include <limits>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>

char const cUnknown  = '0';
char const cExamined = '1';
char const cAsteroid = '#';
char const cNothing  = '.';
char const cNewLine  = '\n';

int gcd(int const aOne, int const aOther) noexcept {
  int absA = abs(aOne);
  int absB = abs(aOther);
  while(absB != 0) {
    int tmp = absB; 
    absB = absA % absB; 
    absA = tmp;
  }
  return absA;
}
        
size_t getMaxVisible(std::deque<std::deque<char>> const &map, int const aWidth, int const aHeight, int const aColumn, int const aRow) {
  std::deque<std::deque<char>> progress;
  for(int i = 0; i < aHeight; ++i) {
    progress.emplace_back(std::deque<char>(aWidth, cUnknown));
  }
  size_t result = 0u;
	for(int row = 0u; row < aHeight; ++row) {
		for(int column = 0u; column < aWidth; ++column) {
      int diffX = column - aColumn;
      int diffY = row - aRow;
      int deltaX = 0;
      int deltaY = 0;
      if(diffX == 0) {
        if(diffY == 0) {
          continue;
        }
        else {
          deltaY = diffY / abs(diffY);
        }
      }
      else {
        if(diffY == 0) {
          deltaX = diffX / abs(diffX);
        }
        else {
          int div = gcd(diffX, diffY);
          deltaX = diffX / div;
          deltaY = diffY / div;
        }
      }
      int nowX, nowY;
      bool found = false;
      for(nowX = aColumn + deltaX, nowY = aRow + deltaY; nowX >= 0 && nowX < aWidth && nowY >= 0 && nowY < aHeight; nowX += deltaX, nowY += deltaY) {
        if(progress[nowY][nowX] == cUnknown) {
          progress[nowY][nowX] = cExamined;
          if(map[nowY][nowX] == cAsteroid) {
            found = true;
          }
          else { // nothing to do
          }
        }
        else { // nothing to do
        }
      }
      result += (found ? 1 : 0);
		}
	}
  return result; 
}

int main(int const argc, char **argv) {
  try {
    if(argc == 1) {
      throw std::invalid_argument("Need input filename.");
    }
    else { // nothing to do
    }
    std::ifstream in(argv[1]);
    std::deque<std::deque<char>> map;
    int width = 0u;
    int height = 0u;
    bool newLine = true;
    while(true) {
      if(newLine) {
        map.emplace_back(std::deque<char>());
        newLine = false;
      }
      else { // nothing to do
      }
      char input = in.get();
      if(input == cAsteroid || input == cNothing) {
        map[height].push_back(input);
      }
      else if(input == cNewLine) {
        width = map[height].size();
        newLine = true;
        ++height;
      }
      if(!in.good()) {
        ++height;
        break;
      }
      else { // nothing to do
      }
    }
    size_t maxAsteroids = 0u;
    for(int row = 0u; row < height; ++row) {
      for(int column = 0u; column < width; ++column) {
        if(map[row][column] == cAsteroid) {
	        maxAsteroids = std::max<size_t>(maxAsteroids, getMaxVisible(map, width, height, column, row));
        }
        else { // nothing to do
        }
      }
    }
    std::cout << maxAsteroids << '\n';
  }
  catch(std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
