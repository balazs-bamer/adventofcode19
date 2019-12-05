#include <iostream>

int main(int argc, char **argv) {
  int lower = 108457;
  int upper = 562041;
  char string[7];
  int count = 0;
  int powersOf10[] = {1, 10, 100, 1000, 10000 };

  string[6] = 0;
  for(int i = lower; i <= upper; ++i) {
    int work = i;
    for(int j = 5; j >= 0; --j) {
      string[j] = work % 10;
      work /= 10;
    }
    bool wasTheSame = false;
    bool wasDecreasing = false;
    for(int j = 0; j < 5; ++j) {
      int diff = string[j] - string[j + 1];
      if(diff == 0) {
        wasTheSame = true;
      }
      else if(diff > 0) {
        wasDecreasing = true;
        if(j < 4) {
          i += diff * powersOf10[4 - j];
          i -= i % powersOf10[4 - j] + 1;
          break;
        }
        else { // nothing to do
        }
      }
      else { // nothing to do
      }
    }
    if(wasTheSame && !wasDecreasing) {
      ++count;
    }
    else { // nothing to do
    }  
  }
  std::cout << count;
}
