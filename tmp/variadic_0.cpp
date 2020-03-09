#include <iostream>
#include <type_traits>
#include <vector>
#define NDEBUG

template <bool a_0>
int reversed_binary_value() {
#ifndef NDEBUG
  std::cout << __PRETTY_FUNCTION__ << ": " << a_0 << std::endl;
#endif
  return a_0;
}

template <bool a_n_minus_1, bool a_n_minus_2, bool... digits>
int reversed_binary_value() {
#ifndef NDEBUG
  std::cout << __PRETTY_FUNCTION__ << ": " << sizeof...(digits) << std::endl;
#endif

  return a_n_minus_1 * (1 << (sizeof...(digits) + 1)) +
         reversed_binary_value<a_n_minus_2, digits...>();
}

using namespace std;
int main() {
  cout << reversed_binary_value<1, 0, 1>() << endl;
  return 0;
}

