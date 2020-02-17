#include <algorithm>
#include <atomic>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <type_traits>
#include <vector>

#define NDEBUG

class Object {
 private:
  signed char value{0};

 public:
  Object(signed char value) : value(value) {
#ifdef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
  }

  bool operator==(const Object &o) const {
#ifdef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
    return value == o.value;
  }

  template <typename T>
  bool operator==(const T &o) const {
#ifdef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
    return value == o;
  }
};
// compile with -fno-elide-constructors to see the moving.

template <typename T, typename K>
T _find(T first, T last, const K &value, std::true_type) {
#ifdef NDEBUG
  std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif

  if (value < std::numeric_limits<signed char>::min() ||
      value > std::numeric_limits<signed char>::max()) {
    return last;
  }
  first = static_cast<T>(memchr(first, value, last - first));
  return first == 0 ? last : first;
}

template <typename T, typename K>
T _find(T first, T last, const K &value, std::false_type) {
#ifdef NDEBUG
  std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif

  for (; first != last; ++first) {
    if (*first == value) {
      break;
    }
  }
  return first;
}

template <typename T, typename K>
T _find(T first, T last, const K &value) {
#ifdef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif    
  typedef std::integral_constant<
      bool, (std::is_same<T, char *>::value ||
             std::is_same<T, signed char *>::value ||
             std::is_same<T, unsigned char *>::value ||
             std::is_same<T, const char *>::value ||
             std::is_same<T, const signed char *>::value ||
             std::is_same<T, const unsigned char *>::value)>
      mem_t;
  return _find<T, K>(first, last, value, mem_t());
}

int main() {
  std::vector<signed char> values{11, 22, 33, -1, 55};
  std::vector<Object> objects{11, 22, 33, -1, 55};

  const signed char *const first = values.data();
  const signed char *const last = values.data() + values.size();

  const Object *const ofirst = objects.data();
  const Object *const olast = objects.data() + values.size();

  auto bug_find = [](const signed char *first, const signed char *last,
                     int value) -> const signed char * {
    first = (const signed char *)memchr(first, value, last - first);
    return first == 0 ? last : first;
  };

  auto my_find = [](const signed char *first, const signed char *last,
                    int value) -> const signed char * {
    if (value < std::numeric_limits<signed char>::min() ||
        value > std::numeric_limits<signed char>::max()) {
      return last;
    }
    first = (const signed char *)memchr(first, value, last - first);
    return first == 0 ? last : first;
  };

  int value = 255UL;

  std::cout << std::find(first, last, value) - first << std::endl;
  std::cout << bug_find(first, last, value) - first << std::endl;
  std::cout << my_find(first, last, value) - first << std::endl;
  std::cout << _find(first, last, value) - first << std::endl;
  std::cout << std::find(ofirst, olast, value) - ofirst << std::endl;
  std::cout << _find(ofirst, olast, value) - ofirst << std::endl;
}
