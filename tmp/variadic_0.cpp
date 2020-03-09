#include <type_traits>
#include <vector>
#include <iostream>
#define NDEBUG

template<bool lsb>
int reversed_binary_value()
{
#ifndef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << ": " << lsb << std::endl;
#endif
    return lsb;
}

template<bool a, bool b, bool... digits>
int reversed_binary_value()
{
#ifndef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << ": " <<  sizeof...(digits) << std::endl;
#endif

    return  a * (1 << (sizeof...(digits) + 1) ) +  reversed_binary_value<b, digits...>();
}

using namespace std;
int main() {
    cout << reversed_binary_value<1, 0, 1>() << endl;
  return 0;
}
