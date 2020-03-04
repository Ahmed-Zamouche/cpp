#include <algorithm>
#include <atomic>
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <type_traits>
#include <vector>

//#define NDEBUG
class Object {
 private:
  friend std::ostream &operator<<(std::ostream &os, const Object &obj);
  static std::atomic<int> ids;

  int id;
  std::string name;

  static void copy(Object &l, const Object &r) {
    if (&l != &r) {
      l.id = ids++;
      l.name = r.name;
    }
  }
  static void move(Object &l, const Object &&r) {
    if (&l != &r) {
      l.name = r.name;
    }
  }

 protected:
  Object(std::string name) : name(name) {
    id = Object::ids++;
#ifndef NDEBUG
    std::cout << "Object `" << toString() << "` created\n";
#endif
  }

  Object() : Object("") {}
  Object &operator=(const Object &o) {
    Object::copy(*this, o);
#ifndef NDEBUG
    if (this != &o) {
      std::cout << "Object `" << toString() << "` copied\n";
    }
#endif
    return *this;
  }
  Object(const Object &o) { *this = o; }

  Object &operator=(Object &&o) {
    Object::move(*this, std::forward<Object &&>(o));
#ifndef NDEBUG
    if (this != &o) {
      std::cout << "Object `" << toString() << "` moved\n";
    }
#endif
    return *this;
  }
  Object(Object &&o) noexcept { *this = std::move(o); };

  bool operator<(const Object &o) const { return id < o.id; };

  virtual std::string toString() const {
    std::string s = "{id: " + std::to_string(id) + ", name: " + name + '}';
    return s;
  };
  virtual ~Object() {
#ifndef NDEBUG
    std::cout << "Object `" << toString() << "` destroyed\n";
#endif
  };
};
std::atomic<int> Object::ids(0);

std::ostream &operator<<(std::ostream &os, const Object &obj) {
  return os << obj.toString();
}

class Base : protected Object {
 private:
 public:
  Base(std::string name) : Object(name) {
#ifndef NDEBUG
    std::cout << "Base `" << toString() << "` created\n";
#endif
  };
  Base() : Base(""){};
  virtual ~Base() {
#ifndef NDEBUG
    std::cout << "Base `" << toString() << "` destroyed\n";
#endif
  };
  virtual std::string toString() const {
    std::string s = "<Base>" + Object::toString();
    return s;
  };
};

std::ostream &operator<<(std::ostream &os, const Base &base) {
  return os << base.toString();
}

class Derived : public Base {
 private:
 public:
  Derived(std::string name) : Base(name) {
#ifndef NDEBUG
    std::cout << "Derived `" << toString() << "` created\n";
#endif
  };
  Derived() : Derived(""){};
  virtual ~Derived() {
#ifndef NDEBUG
    std::cout << "Derived `" << toString() << "` destroyed\n";
#endif
  };
  virtual std::string toString() const {
    std::string s = "<Derived>" + Object::toString();
    return s;
  };
};

std::ostream &operator<<(std::ostream &os, const Derived &derived) {
  return os << derived.toString();
}

using std::cout;
using std::endl;

template <typename T>
std::unique_ptr<T> factory(const std::string &name,
                           std::true_type) {  // source: produces T
  std::cout << __PRETTY_FUNCTION__ << std::endl;
  return std::make_unique<T>(name);
}

template <typename T>
std::shared_ptr<T> factory(
    const std::string &name,
    std::false_type) {  // source: produces T. share perhaps by factory itself
  std::cout << __PRETTY_FUNCTION__ << std::endl;
  return std::make_shared<T>(name);
}

template <typename T>
T factory(const std::string &name) {
  std::cout << __PRETTY_FUNCTION__ << std::endl;

  typedef typename std::remove_reference<decltype(*T())>::type PointerType;
  typedef std::integral_constant<
      bool, std::is_same<T, std::unique_ptr<PointerType>>::value>
      SmartPointerType;

  return factory<PointerType>(name, SmartPointerType());
}

template <typename T>
void reseat(std::unique_ptr<T> &t) {  // will or might reseat ptr
  std::cout << __PRETTY_FUNCTION__ << std::endl;
  std::unique_ptr<Base> tmp = factory<std::unique_ptr<Base>>("base_1");
  std::cout << "reseat: " << *t << " -> " << *tmp << std::endl;
  t.swap(tmp);
}

template <typename T>
void sink(std::unique_ptr<T> t) {  // sink: consumes T
  std::cout << __PRETTY_FUNCTION__ << std::endl;
  std::cout << "sink: " << *t << std::endl;
}

template <typename T>
void reseat(std::shared_ptr<T> &t) {  // will or might reseat ptr
  std::cout << __PRETTY_FUNCTION__ << std::endl;
  std::shared_ptr<Base> tmp = factory<std::shared_ptr<Base>>("base_1");
  std::cout << "reseat: " << *t << ", use_count: " << t.use_count() << " -> "
            << *tmp << ", use_count: " << tmp.use_count() << std::endl;
  t.swap(tmp);
}

template <typename T>
void share(std::shared_ptr<T> t) {  // share : retain ref count
  std::cout << __PRETTY_FUNCTION__ << std::endl;
  std::cout << *t << ", use_count: " << t.use_count() << std::endl;
  std::shared_ptr<Base> tt = t;
  std::cout << *tt << ", use_count: " << tt.use_count() << std::endl;
}

void example_unique() {
  std::unique_ptr<Base> base = factory<std::unique_ptr<Base>>("base_0");
  std::cout << *base << std::endl;
  reseat<Base>(base);
  std::cout << *base << std::endl;
  sink<Base>(std::move(base));
  std::cout << *base << std::endl;
  assert(base == nullptr);
}

void example_shared() {
  std::shared_ptr<Base> base = factory<std::shared_ptr<Base>>("base_0");
  std::shared_ptr<Base> _base = base;
  std::cout << *base << ", use_count: " << base.use_count() << std::endl;
  share<Base>(base);
  std::cout << *base << ", use_count: " << base.use_count() << std::endl;
  reseat<Base>(base);

  std::cout << *base << ", use_count: " << base.use_count() << std::endl;
  std::cout << *_base << ", use_count: " << _base.use_count() << std::endl;
  assert(base != nullptr);
}

int main() {
  example_unique();
  example_shared();

  return 0;
}
