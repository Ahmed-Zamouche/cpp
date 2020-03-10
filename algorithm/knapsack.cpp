/*
 * Generic 01 Knapsack Problem  solver:
 *
 */
#include <array>
#include <iostream>

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
  return os << "ptr: " << knapsackItem.ptr << ", value: " << knapsackItem.value
            << ", space: " << knapsackItem.space;
}

// Top-Down with Memoization
template <typename T, typename TValue, typename TSpace, size_t N = 0>
TValue knapsack_recursive(std::array<Knapsack<T, TValue, TSpace>, N> &A, int i,
                          TSpace capacity) {
  if (capacity <= 0) {
    return TValue{};
  }
  if (i == 0) {
    return (*A[0].space <= capacity) ? *A[0].value : TValue{};
  }
#ifndef NDEBUG
  std::cout << i << ':' << ", {" << A[i] << "}, capacity: " << capacity
            << std::endl;
#endif
  return std::max(
      knapsack_recursive(A, i - 1, capacity),
      *A[i].value + knapsack_recursive(A, i - 1, capacity - *A[i].space));
}

// Bottom-Up wit Tabbulation
template <typename T, typename TValue, typename TSpace, size_t N = 0>
TValue knapsack_iterative(std::array<Knapsack<T, TValue, TSpace>, N> &A,
                          TSpace capacity) {
  for (size_t i = 0; i < A.size(); ++i) {
  }

  return TValue{0};
}

const bool KNAPSACK_USE_RECURSIVE = true;

template <typename T, typename TValue, typename TSpace, size_t N = 0>
TValue knapsack(std::array<Knapsack<T, TValue, TSpace>, N> &A,
                TSpace capacity) {
  if (KNAPSACK_USE_RECURSIVE) {
    return knapsack_recursive(A, A.size() - 1, capacity);
  } else {
    return knapsack_iterative(A, capacity);
  }
}

template <typename T, typename TValue, typename TSpace, size_t N = 0>
TValue knapsack(std::array<T, N> &A, TSpace capacity) {
  std::array<Knapsack<T, TValue, TSpace>, N> knapsackItems;
  for (size_t i = 0; i < A.size(); ++i) {
    knapsackItems[i] =
        std::move(Knapsack<T, int, int>(&A[i].value, &A[i].space, &A[i]));
  }
  return knapsack<T, TValue, TSpace, N>(knapsackItems, capacity);
}

struct Item {
  int value{};
  int space{};
  bool operator<(const Item &o) const { return value < o.value; }
};
std::ostream &operator<<(std::ostream &os, const Item &item) {
  return os << "value: " << item.value << ", space: " << item.space;
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

  int capacity = 17;

  cout << knapsack<Item, int, int>(items, capacity) << endl;
}
