//House Robber algorithm
#include <array>
#include <cstring>
#include <iostream>

//#define NDEBUG

// Top-Down with Memoization
template <typename T, size_t N = 0>
T rob_recursive(std::array<T, N> &A, int i, std::array<T, N> &cache) {
  if (i < 0) {
    return 0;
  }
  if (cache[i] < 0) {
    cache[i] = std::max(rob_recursive(A, i - 2, cache) + A[i],
                        rob_recursive(A, i - 1, cache));
  }

  return cache[i];
}

// Bottom-Up wit Tabbulation
template <typename T, size_t N = 0>
T rob_iterative(std::array<T, N> &A) {
  T v_i_minus_2 = A[0];
  T v_i_minus_1 = std::max(A[1], A[0]);

  for (size_t i = 2; i < A.size(); ++i) {
    T v_i = std::max(v_i_minus_2 + A[i], v_i_minus_1);
    v_i_minus_2 = v_i_minus_1;
    v_i_minus_1 = v_i;
  }

  return std::max(v_i_minus_2, v_i_minus_1);
}

const bool ROB_USE_RECURSIVE = true;

template <typename T, size_t N = 0>
T rob(std::array<T, N> &A) {
  if (ROB_USE_RECURSIVE) {
    std::array<T, N> cache;
    for (auto &&e : cache) {
      e = -1;
    }
    cache[0] = A[0];
    cache[1] = std::max(A[1], A[0]);
    return rob_recursive(A, (A.size() - 1), cache);
  } else {
    return rob_iterative(A);
  }
}

using std::cin;
using std::cout;
using std::endl;

int main() {
  std::array<int, 25> A{1, 0, 0, 2, 7, 5, 7, 0, 4, 7, 4, 9, 2,
                        6, 3, 1, 5, 4, 2, 0, 1, 6, 5, 2, 2};

  for (auto &&a : A) {
    cout << a << ',';
  }
  cout << endl;

  cout << rob(A) << endl;
}
