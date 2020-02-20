//trp: Type Rich Programming
//from: GoingNative 2012 Day 1 Keynote - Bjarne Stroustrup: C++11 Style
//see: https://channel9.msdn.com/Events/GoingNative/GoingNative-2012/Keynote-Bjarne-Stroustrup-Cpp11-Style
#include <cmath>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

template <int M, int K, int S>
struct Unit {
  enum { m = M, kg = K, s = S };
};

template <typename U, typename T>
struct Value {
  double value;
  explicit Value(const T &value) : value(value) {}
  std::string to_string() const {
    std::string str = std::to_string(value) + " [";

    if (U::kg != 0) {
      str += "kg^" + std::to_string(U::kg);
    }

    if (U::m != 0) {
      str += "m^" + std::to_string(U::m);
    }

    if (U::s != 0) {
      str += "s^" + std::to_string(U::s);
    }
    str += "]";
    return str;
  }
};

template <typename U, typename T>
Value<U, T> operator+(const Value<U, T> &l, const Value<U, T> &r) {
  return Value<U, T>(l.value + r.value);
}

template <typename U, typename T>
void operator+=(const Value<U, T> &l, const Value<U, T> &r) {
  l.value += r.value;
}

template <typename U, typename T>
Value<U, T> operator-(const Value<U, T> &l, const Value<U, T> &r) {
  return Value<U, T>(l.value - r.value);
}

template <typename U, typename T>
void operator-=(const Value<U, T> &l, const Value<U, T> &r) {
  l.value -= r.value;
}

template <typename Ul, typename Ur, typename T>
Value<Unit<Ul::m + Ur::m, Ul::kg + Ur::kg, Ul::s + Ur::s>, T> operator*(
    const Value<Ul, T> &l, const Value<Ur, T> &r) {
  return Value<Unit<Ul::m + Ur::m, Ul::kg + Ur::kg, Ul::s + Ur::s>, T>(l.value *
                                                                       r.value);
}

template <typename U, typename T>
Value<U, T> operator*(const double &l, const Value<U, T> &r) {
  return Value<Unit<0, 0, 0>, T>(l) * r;
}

template <typename U, typename T>
Value<U, T> operator*(const Value<U, T> &l, const double &r) {
  return l * Value<Unit<0, 0, 0>, T>(r);
}

template <typename U, typename T>
void operator*=(const Value<U, T> &l, const double &r) {
  l.value *= r;
}

template <typename Ul, typename Ur, typename T>
Value<Unit<Ul::m - Ur::m, Ul::kg - Ur::kg, Ul::s - Ur::s>, T> operator/(
    const Value<Ul, T> &l, const Value<Ur, T> &r) {
  return Value<Unit<Ul::m - Ur::m, Ul::kg - Ur::kg, Ul::s - Ur::s>, T>(l.value /
                                                                       r.value);
}

template <typename U, typename T>
Value<Unit<-U::m, -U::kg, -U::s>, T> operator/(const double &l,
                                               const Value<U, T> &r) {
  return Value<Unit<0, 0, 0>, T>(l) / r;
}

template <typename U, typename T>
Value<U, T> operator/(const Value<U, T> &l, const double &r) {
  return l / Value<Unit<0, 0, 0>, T>(r);
}

template <typename U, typename T>
void operator/=(const Value<U, T> &l, const double &r) {
  l.value /= r;
}

using Meter = Unit<1, 0, 0>;
using Kilogram = Unit<0, 1, 0>;
using Second = Unit<0, 0, 1>;

Value<Meter, double> operator"" _m(long double value) {
  return Value<Meter, double>(value);
}
Value<Kilogram, double> operator"" _kg(long double value) {
  return Value<Kilogram, double>(value);
}
Value<Second, double> operator"" _s(long double value) {
  return Value<Second, double>(value);
}

/*
using Foot = Unit<1, 0, 0>;
using Pound = Unit<0, 1, 0>;
using Second = Unit<0, 0, 1>;
*/

using Length = Value<Meter, double>;
using Mass = Value<Kilogram, double>;
using Time = Value<Second, double>;

using Speed = Value<Unit<1, 0, -1>, double>;
using Acceleration = Value<Unit<1, 0, -2>, double>;
using Force = Value<Unit<1, 1, -2>, double>;

using namespace std;

int main() {
  Length l0 = 3.14_m;
  Time t0 = 13.9_s;
  Mass m0 = 1.4_kg;

  Speed sp1 = l0 / t0;

  Acceleration a0 = sp1 / 3.0_s;

  Force f0 = m0 * a0;

  cout << l0.to_string() << endl;
  cout << t0.to_string() << endl;
  cout << m0.to_string() << endl;

  cout << sp1.to_string() << endl;
  cout << a0.to_string() << endl;
  cout << f0.to_string() << endl;

  cout << (f0 / 2).to_string() << endl;
  cout << (2 / f0).to_string() << endl;
  cout << (f0 / f0).to_string() << endl;

  cout << (f0 * 2).to_string() << endl;
  cout << (2 * f0).to_string() << endl;
  cout << (f0 * f0).to_string() << endl;

  cout << (3.14_m).to_string() << endl;
  cout << (2.5_kg).to_string() << endl;
  cout << (13.7_s).to_string() << endl;

  // Speed sp1 = Value<M>(100)/Value<S>(100);
}
