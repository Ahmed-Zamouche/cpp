#include <algorithm>
#include <atomic>
#include <cmath>
//#include <functional>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <vector>
#include <cassert>

//#define NDEBUG

class Object {

private:
	static std::atomic<int> ids;

	Object& copy(const Object &o) {
		if (this != &o) {
			name = o.name;
#ifndef NDEBUG
			std::cout << "Object `" << to_string() << "` copied\n";
#endif
		}
		return *this;
	}

	Object& move(Object &&o) {
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
	std::string name { "noname" };

public:

	Object(const std::string &name) :
			name(name) {
#ifndef NDEBUG
		std::cout << "Object(std::string &) `" << to_string() << "` created\n";
#endif
	}

	Object() {
#ifndef NDEBUG
		std::cout << "Object() `" << to_string() << "` created\n";
#endif
	}

	Object& operator=(const Object &o) {
		return copy(o);
	}

	Object(const Object &o) {
		copy(o);
	}

	Object& operator=(Object &&o) {
		return move(std::forward<Object&&>(o));
	}

	Object(Object &&o) noexcept {
		move(std::forward<Object&&>(o));
	}

	bool operator<(const Object &o) const {
		return id < o.id;
	}

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

std::ostream& operator<<(std::ostream &os, const Object &obj) {
	return os << obj.to_string();
}

class Base: public Object {

private:
	static const int SIZE = 32;
	int *buffer = nullptr;

	Base& copy(const Base &o) {
		if (this != &o) {
			if (buffer == nullptr) {
				buffer = new int[Base::SIZE];
			}
			memcpy(buffer, o.buffer, Base::SIZE * sizeof(int));
#ifndef NDEBUG
			std::cout << "Base `" << to_string() << "` copied\n";
#endif
		}
		return *this;
	}

	Base& move(Base &&o) {
		if (this != &o) {
			int *ptr = buffer;
			buffer = o.buffer;
			//delegate the deletion to other `o`
			o.buffer = ptr;
#ifndef NDEBUG
			std::cout << "Base `" << to_string() << "` moved\n";
#endif
		}
		return *this;
	}

protected:

public:

	Base(const std::string &name) :
			Object(name) {
		buffer = new int[Base::SIZE]();
#ifndef NDEBUG
		std::cout << "Base(std::string &) `" << to_string() << "` created\n";
#endif
	}

	Base() {
		buffer = new int[Base::SIZE]();
#ifndef NDEBUG
		std::cout << "Base() `" << to_string() << "` created\n";
#endif
	}

	Base& operator=(const Base &o) {
		Object::operator=(o);
		return copy(o);
	}

	Base(const Base &o) :
			Object(o) {
		copy(o);
	}

	Base& operator=(Base &&o) {
		Object::operator=(std::forward<Base&&>(o));
		return move(std::forward<Base&&>(o));
	}

	Base(Base &&o) noexcept :
			Object(std::move(o)) {
		move(std::forward<Base&&>(o));
	}

	virtual ~Base() {
#ifndef NDEBUG
		std::cout << "Base `" << to_string() << "` destroyed\n";
#endif
		delete[] buffer;
		buffer = nullptr;
	}

	virtual std::string to_string() const {
		std::stringstream ss;
		ss << "Base: {" << Object::to_string() << ", buffer: " << buffer
				<< +"}";
		return ss.str();
	}

};

template<class T>
class Derived: public Base {

private:
	T value { };

	Derived& copy(const Derived &o) {
		if (this != &o) {
			value = o.value;
#ifndef NDEBUG
			std::cout << "Derived `" << to_string() << "` copied\n";
#endif
		}
		return *this;
	}

	Derived& move(Derived &&o) {
		if (this != &o) {
			value = o.value;
#ifndef NDEBUG
			std::cout << "Derived `" << to_string() << "` moved\n";
#endif
		}
		return *this;
	}

public:

	Derived(const std::string &name) :
			Base(name) {
#ifndef NDEBUG
		std::cout << "Derived(std::string &) `" << to_string() << "` created\n";
#endif
	}

	Derived() {
#ifndef NDEBUG
		std::cout << "Derived() `" << to_string() << "` created\n";
#endif
	}

	Derived& operator=(const Derived &o) {
		Base::operator=(o);
		return copy(o);
	}

	Derived(const Derived &o) :
			Base(o) {
		copy(o);
	}

	Derived& operator=(Derived &&o) {
		Base::operator=(std::forward<Derived&&>(o));
		return move(std::forward<Derived&&>(o));
	}

	Derived(Derived &&o) noexcept :
			Base(std::move(o)) {
		move(std::forward<Derived&&>(o));
	}

	Derived operator++(int) { // Postfix
		Derived r = *this;
#ifndef NDEBUG
		std::cout << "    Derived++ `" << to_string() << "`\n";
#endif
		value++;
		return r;
	}

	Derived& operator++() { // Prefix
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
	// int choice = 5;

	Base b("base");

	Derived<int> dInt0("derived_int");
	Derived<float> dFloat0("derived_float");

	std::cout << '\n';
	++dInt0;
	++dInt0;
	++dFloat0;
	std::cout << '\n';

	// dangerous
	std::cout << "static cast" << '\n';
	Derived<int> *pDi0 = static_cast<Derived<int>*>(&b);
	Derived<float> *pDf = static_cast<Derived<float>*>(&b);

	Base *pB;
	pB = &dInt0;
	Derived<float> *pDif = static_cast<Derived<float>*>(pB);
	pB = &dFloat0;
	Derived<int> *pDfi = static_cast<Derived<int>*>(pB);

	std::cout << *pB << '\n';
	std::cout << *pDi0 << '\n';
	std::cout << *pDf << '\n';
	std::cout << *pDif << '\n';
	std::cout << *pDfi << '\n';
	std::cout << '\n';

	// make sure to remove  -Werror
	std::cout << "dynamic cast" << '\n';
	Derived<int> *pDi1;
	pDi1 = dynamic_cast<Derived<int>*>(&b);
	assert(pDi1 == nullptr);
	pDi1 = dynamic_cast<Derived<int>*>(&dFloat0);
	assert(pDi1 == nullptr);

	pB = dynamic_cast<Base*>(&b);
	pDi1 = dynamic_cast<Derived<int>*>(&dInt0);

	std::cout << '\n';
	std::cout << *pB << '\n';
	std::cout << *pDi1 << '\n';
	std::cout << '\n';

	// dangerous use static_cast or dynamic_cast first
	std::cout << "reinterpret_cast: C style cast" << '\n';

	// Assing left value using static cast
	Base &&d0 = Derived<int>("derived_int_0");
	Base &&d1 = static_cast<Base&&>(dFloat0);

	std::cout << '\n';
	std::cout << d0 << '\n';
	std::cout << d1 << '\n';
	std::cout << '\n';

	std::cout << "--------\n";
	return 0;
}
