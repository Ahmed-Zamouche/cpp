//Dynamic Programming – Stairs Climbing Puzzle
#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

using std::cin;
using std::cout;
using std::endl;

// Bottom-Up wit Tabbulation
template <typename T>
void climbing_stairs_iterative(T n, T m, T *cache) {
  /*
      a[0] = 1; //1 step  -> 2^0
      a[1] = 2; //2 steps -> 2^1
      a[2] = 4; //3 steps -> 2^2
      a[3] = 8; //4 steps -> 2^3
      a[4] = 8; //5 steps -> 2^4
  */
  for (T i = 0; i < m; ++i) {
    cache[i] = (1 << i);
  }
  // a[n] = a[n-1] + a[n-2] + a[n-3] + ... + a[n-m];
  for (T i = m; i < n; ++i) {
    cache[i] = 0;
    for (T j = 0; j < m; ++j) {
      cache[i] += cache[i - j -1];
    }
  }
}

template <typename T>
T * climbing_stairs(T n, T m) {
  T *cache = new T[n];
  climbing_stairs_iterative(n, m, cache);
  return cache;
}

using namespace std;
int main() {
    //example of climbing 64 stairs with at most 4 steps
    long long n = 64, m=4; 
    long long *cache = climbing_stairs(n, m);
    for(long long i = 0; i < n; ++i){
        cout << i << ": "<< cache [i]<< endl;
    }
    return 0; 
}
