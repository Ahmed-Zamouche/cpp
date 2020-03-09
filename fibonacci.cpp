#include <iostream>
#include <cstring>

//#define NDEBUG

//Top-Down with Memoization
template <typename T>
T fib_recursive(T n, T *cache){
    //std::cout << __PRETTY_FUNCTION__ << ": n="<< n << std::endl;
    if(n < 2){
        cache[n] = n;
        return n;
    }
    if(cache[n] < 0){
        cache[n] = fib_recursive(n -1, cache) + fib_recursive(n - 2, cache);
    }

    return cache[n];
}

template <typename T>
void fib_iterative(T n, T *cache){
    cache[0] = 0;
    cache[1] = 1;
    for(T i = 2; i < n; i++){
        cache[i] =cache[i-1] + cache[i-2];
    }
}

template <typename T>
T *fib(T n){
   T *cache = new T[n + 1];
   memset(cache, -1, (n + 1) * sizeof(T));    
   fib_iterative(n, cache);
   return cache;
}

//#define USE_BENCHMARK
#ifdef USE_BENCHMARK

static long long n =  25;//max 92;
long long *cache = new long long[n + 1];

#include <benchmark/benchmark.h>
static void bm_fib_recursive(benchmark::State& state) {
  for (auto _ : state) {
    memset(cache, -1, (n + 1) * sizeof(long long));
    fib_recursive<long long>(n, cache);
  }
}
// Register the function as a benchmark
BENCHMARK(bm_fib_recursive);

static void bm_fib_iterative(benchmark::State& state) {
  for (auto _ : state) {
      fib_iterative<long long>(n, cache);
  }
}
BENCHMARK(bm_fib_iterative);

BENCHMARK_MAIN();

#else
using std::cout;
using std::cin;
using std::endl;


int main(){

   long long n = 100;
   long long *cache = fib<long long>(n);

    for(long long i  = 0; i < n; i++){
        cout << i << ":" << cache[i] << endl;
    }
    cout <<  endl;
}
#endif
