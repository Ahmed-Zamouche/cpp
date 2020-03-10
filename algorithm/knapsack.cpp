/*
 * Generic 01 Knapsack Problem  solver:
 *
 */
#include <array>
#include <iostream>
#include <memory>

//#define NDEBUG

/**
 * Knapsack Type used a wrapper for the target Type which we are solving
 * 01 Knapsack for. The target type MUST have
 * Value property that is to be maximized
 * space property used as constraint  to the maximum capacity.
 * ptr optionally a pointer to the target type used for debugging
 */
template <typename T, typename TValue, typename TSpace>
struct Knapsack {
  TValue *value{};
  TSpace *space{};
  T *ptr{};
  Knapsack() {}
  Knapsack(TValue *value, TSpace *space, T *ptr)
      : value(value), space(space), ptr(ptr) {}
  Knapsack<T, TValue, TSpace> &operator=(Knapsack<T, TValue, TSpace> &&o) {
    if (this != &o) {
      value = std::move(o.value);
      space = std::move(o.space);
      ptr = std::move(o.ptr);
    }
    return *this;
  }
};

template <typename T, typename TValue, typename TSpace>
std::ostream &operator<<(std::ostream &os,
                         const Knapsack<T, TValue, TSpace> &knapsackItem) {
  return os << *knapsackItem.ptr;
}

// Top-Down with Memoization
template <typename T, typename TValue, typename TSpace, size_t N = 0>
TValue knapsack_recursive(std::array<Knapsack<T, TValue, TSpace>, N> &A, int i,
                          TSpace c, std::unique_ptr<TValue> &cache) {
  if (c <= 0) {
    return TValue{};
  }
  if (i == 0) {
    return (*A[0].space <= c) ? *A[0].value : TValue{};
  }
#ifndef NDEBUG
  std::cout << i << ':' << ", {" << A[i] << "}, capacity: " << c << std::endl;
#endif
  TValue *p_cache = cache.get();
  if (p_cache[i - 1 + N * c] < 0) {
    p_cache[i - 1 + N * c] = knapsack_recursive(A, i - 1, c, cache);
  }
  int c_minus_space_i = c - *A[i].space;

  if (p_cache[i + N * c_minus_space_i] < 0) {
    p_cache[i + N * c_minus_space_i] =
        *A[i].value + knapsack_recursive(A, i - 1, c_minus_space_i, cache);
  }

  return std::max(p_cache[i - 1 + N * c], p_cache[i + N * c_minus_space_i]);
}

// Bottom-Up with Tabbulation
template <typename T, typename TValue, typename TSpace, size_t N = 0>
TValue knapsack_iterative(std::array<Knapsack<T, TValue, TSpace>, N> &A,
                          TSpace capacity, std::unique_ptr<TValue> &cache) {
  TValue *p_cache = cache.get();

  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j <= capacity; ++j) {
      if (i == 0 || j == 0) {
        p_cache[i + N * j] = TValue{};
      }
      if (*A[i].space > j) {
        p_cache[i + N * j] = p_cache[i - 1 + N * j];
      } else {
        int c_minus_space_i = j - *A[i].space;
        p_cache[i + N * j] =
            std::max(*A[i].value + p_cache[i - 1 + N * c_minus_space_i],
                     p_cache[i - 1 + N * j]);
      }
#ifndef NDEBUG
      std::cout << i << ':' << ", {" << A[i] << "}, capacity: " << j << ", "
                << p_cache[i + N * j] << std::endl;
#endif
    }
  }

  return p_cache[N - 1 + N * capacity];
}

const bool KNAPSACK_USE_RECURSIVE = false;

template <typename T, typename TValue, typename TSpace, size_t N = 0>
TValue knapsack(std::array<Knapsack<T, TValue, TSpace>, N> &A, TSpace C) {
  if (KNAPSACK_USE_RECURSIVE) {
    std::unique_ptr<TValue> cache = std::make_unique<TValue>(N * C);
    for (size_t i = 0; i < N; ++i) {
      for (size_t j = 0; j < C; ++j) {
        cache.get()[i + N * j] = TValue{-1};
      }
    }
    return knapsack_recursive(A, N - 1, C, cache);
  } else {
    std::unique_ptr<TValue> cache = std::make_unique<TValue>(N * (C + 1));
    return knapsack_iterative(A, C, cache);
  }
}

struct Item {
  int _value{};
  int _space{};
  bool operator<(const Item &o) const { return _value < o._value; }
};
std::ostream &operator<<(std::ostream &os, const Item &item) {
  return os << "value: " << item._value << ", space: " << item._space;
}
using std::cin;
using std::cout;
using std::endl;

int main() {
  std::array<Item, 5> items = {
      Item{4, 12}, {2, 2}, {2, 1}, {1, 1}, {10, 4}};  //

  for (auto &&item : items) {
    cout << '{' << item << '}' << endl;
  }
  cout << endl;
  // wrap Item with Knapsack an invoke the algorithm
  std::array<Knapsack<Item, int, int>, 5> knapsackItems;
  for (size_t i = 0; i < items.size(); ++i) {
    knapsackItems[i] = std::move(Knapsack<Item, int, int>(
        &items[i]._value, &items[i]._space, &items[i]));
  }

  int capacity = 5;

  cout << knapsack(knapsackItems, capacity) << endl;
}
