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
#include <vector>

//#define NDEBUG

class Object {
 private:
  static std::atomic<int> ids;

  Object &copy(const Object &o) {
    if (this != &o) {
      name = o.name;
#ifndef NDEBUG
      std::cout << "Object `" << to_string() << "` copied\n";
#endif
    }
    return *this;
  }

  Object &move(Object &&o) {
    if (this != &o) {
      name = std::move(o.name);
#ifndef NDEBUG
      std::cout << "Object `" << to_string() << "` moved\n";
#endif
    }
    return *this;
  }

 protected:
  int id = Object::ids++;
  std::string name{"noname"};

 public:
  Object(const std::string &name) : name(name) {
#ifndef NDEBUG
    std::cout << "Object(std::string &) `" << to_string() << "` created\n";
#endif
  }

  Object() {
#ifndef NDEBUG
    std::cout << "Object() `" << to_string() << "` created\n";
#endif
  }

  Object &operator=(const Object &o) { return copy(o); }

  Object(const Object &o) { copy(o); }

  Object &operator=(Object &&o) { return move(std::forward<Object &&>(o)); }

  Object(Object &&o) noexcept { move(std::forward<Object &&>(o)); }

  bool operator<(const Object &o) const { return id < o.id; }

  virtual std::string to_string() const {
    std::stringstream ss;
    ss << "Object: {id: " << id << ", name: " << name << '}';
    return ss.str();
  }

  virtual ~Object() {
#ifndef NDEBUG
    std::cout << "Object `" << to_string() << "` destroyed\n";
#endif
  }
};

std::atomic<int> Object::ids(0);

std::ostream &operator<<(std::ostream &os, const Object &obj) {
  return os << obj.to_string();
}

class Foo : public Object {
 private:
  static std::atomic<int> ordinals;
  int ordinal{Foo::ordinals++};
  int value{ordinal};

  Foo &copy(const Foo &o) {
    if (this != &o) {
      value = o.value;
#ifndef NDEBUG
      std::cout << "Foo `" << to_string() << "` copied\n";
#endif
    }
    return *this;
  }

  Foo &move(Foo &&o) {
    if (this != &o) {
      value = std::move(o.value);
#ifndef NDEBUG
      std::cout << "Foo `" << to_string() << "` moved\n";
#endif
    }
    return *this;
  }

 public:
  Foo() : Object("foo") {
#ifndef NDEBUG
    std::cout << "Foo `" << to_string() << "` created\n";
#endif
  }
  Foo &operator=(const Foo &o) {
    Object::operator=(o);
    return copy(o);
  }

  Foo(const Foo &o) : Object(o) { copy(o); }

  Foo &operator=(Foo &&o) {
    Object::operator=(std::forward<Foo &&>(o));
    return move(std::forward<Foo &&>(o));
  }

  Foo(Foo &&o) noexcept : Object(std::move(o)) {
    move(std::forward<Foo &&>(o));
  }

  ~Foo() {
#ifndef NDEBUG
    std::cout << "Foo `" << to_string() << "` destroyed\n";
#endif
  }
  virtual std::string to_string() const {
    std::stringstream ss;
    ss << "Foo: {" << Object::to_string() << ", ordinal: " << ordinal
       << ", value: " << value << '}';
    return ss.str();
  }
};

std::atomic<int> Foo::ordinals(0);

class Base : public Object {
 private:
  static const int SIZE = 2;
  std::unique_ptr<Foo[]> foos;

  Base &copy(const Base &o) {
    if (this != &o) {
      if (foos.get() == nullptr) {
        // foos = std::make_unique<Foo[]>(Base::SIZE);
        foos = std::unique_ptr<Foo[]>(new Foo[Base::SIZE]);
      }
      for (int i = 0; i < Base::SIZE; i++) {
        foos.get()[i] = o.foos.get()[i];
      }
#ifndef NDEBUG
      std::cout << "Base `" << to_string() << "` copied\n";
#endif
    }
    return *this;
  }

  Base &move(Base &&o) {
    if (this != &o) {
      foos.swap(o.foos);
#ifndef NDEBUG
      std::cout << "Base `" << to_string() << "` moved\n";
#endif
    }
    return *this;
  }

 protected:
 public:
  Base(const std::string &name) : Object(name) {
    // foos = std::make_unique<Foo[]>(Base::SIZE);
    foos = std::unique_ptr<Foo[]>(new Foo[Base::SIZE]);
#ifndef NDEBUG
    std::cout << "Base(std::string &) `" << to_string() << "` created\n";
#endif
  }

  Base() {
    // foos = std::make_unique<Foo[]>(Base::SIZE);
    foos = std::unique_ptr<Foo[]>(new Foo[Base::SIZE]);
#ifndef NDEBUG
    std::cout << "Base() `" << to_string() << "` created\n";
#endif
  }

  Base &operator=(const Base &o) {
    Object::operator=(o);
    return copy(o);
  }

  Base(const Base &o) : Object(o) { copy(o); }

  Base &operator=(Base &&o) {
    Object::operator=(std::forward<Base &&>(o));
    return move(std::forward<Base &&>(o));
  }

  Base(Base &&o) noexcept : Object(std::move(o)) {
    move(std::forward<Base &&>(o));
  }

  virtual ~Base() {
#ifndef NDEBUG
    std::cout << "Base `" << to_string() << "` destroyed\n";
#endif
  }

  virtual std::string to_string() const {
    std::stringstream ss;
    ss << "Base: {" << Object::to_string() << ", foos: " << foos.get() << +"}";
    return ss.str();
  }
};

template <class T>
class Derived : public Base {
 private:
  T value{};

  Derived &copy(const Derived &o) {
    if (this != &o) {
      value = o.value;
#ifndef NDEBUG
      std::cout << "Derived `" << to_string() << "` copied\n";
#endif
    }
    return *this;
  }

  Derived &move(Derived &&o) {
    if (this != &o) {
      value = o.value;
#ifndef NDEBUG
      std::cout << "Derived `" << to_string() << "` moved\n";
#endif
    }
    return *this;
  }

 public:
  Derived(const std::string &name) : Base(name) {
#ifndef NDEBUG
    std::cout << "Derived(std::string &) `" << to_string() << "` created\n";
#endif
  }

  Derived() {
#ifndef NDEBUG
    std::cout << "Derived() `" << to_string() << "` created\n";
#endif
  }

  Derived &operator=(const Derived &o) {
    Base::operator=(o);
    return copy(o);
  }

  Derived(const Derived &o) : Base(o) { copy(o); }

  Derived &operator=(Derived &&o) {
    Base::operator=(std::forward<Derived &&>(o));
    return move(std::forward<Derived &&>(o));
  }

  Derived(Derived &&o) noexcept : Base(std::move(o)) {
    move(std::forward<Derived &&>(o));
  }

  Derived operator++(int) {  // Postfix
    Derived r = *this;
#ifndef NDEBUG
    std::cout << "    Derived++ `" << to_string() << "`\n";
#endif
    value++;
    return r;
  }

  Derived &operator++() {  // Prefix
    ++value;
#ifndef NDEBUG
    std::cout << "    ++Derived `" << to_string() << "`\n";
#endif
    return *this;
  }

  virtual ~Derived() {
#ifndef NDEBUG
    std::cout << "Derived `" << to_string() << "` destroyed\n";
#endif
  }

  virtual std::string to_string() const {
    std::stringstream ss;
    ss << "Derived: {" << Base::to_string() << ", value: " << value << "}";
    return ss.str();
  }
};

// -std=c++11 -Wall -Wextra  -Werror -pedantic -pedantic-errors
// -fno-elide-constructors

int main() {
#if 0
  // check that copy and move constructor and assignment  works
  Derived<int> d0("d0");
  std::cout << d0 << '\n';
  std::cout << '\n';

  std::cout << "Copy constructor\n";
  Derived<int> d1 = d0;
  std::cout << d1 << '\n';
  std::cout << '\n';

  std::cout << "Move constructor\n";
  Derived<int> d2 = std::move(d0);
  std::cout << d2 << '\n';
  std::cout << '\n';

  std::cout << "Copy assignment\n";
  d2 = d1;
  std::cout << d2 << '\n';
  std::cout << '\n';

  std::cout << "Move assignment\n";
  d0 = std::move(d1);
  std::cout << d0 << '\n';
  std::cout << '\n';

  std::cout << "--------\n";
  return 0;
#endif

  std::unique_ptr<Derived<int>> pD0(new Derived<int>("alloc_derived_0"));
  std::cout << '\n';
  {
    std::unique_ptr<Derived<int>> pD1(new Derived<int>("alloc_derived_1"));
    std::cout << '\n';
    {
      std::unique_ptr<Derived<int>> pD2(new Derived<int>("alloc_derived_2"));
      std::cout << '\n';
      std::cout << *pD2 << '\n';
      std::cout << '\n';
    }
    std::cout << *pD1 << '\n';
    std::cout << '\n';
  }
  std::cout << *pD0 << '\n';
  std::cout << '\n';

  std::unique_ptr<Derived<int>[]> pDs(new Derived<int>[2]);
  std::cout << "--------\n";
  return 0;
}
