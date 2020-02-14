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

class Derived: public Base {

private:
	int value { 0 };

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

std::unique_ptr<Derived> foo() {
	std::unique_ptr<Derived> ptr { new Derived("unique_ptr") };
	return ptr;
}

int& bar() {
	static int s_int = 0;
	return s_int;
}

void baz_value(const Derived &derived) {
	std::cout << "@@@--------------------------------@@@\n";
	std::cout << "lvalue: " << derived << '\n';
	std::cout << "@@@--------------------------------@@@\n";
}

void baz_value(Derived &&derived) {
	std::cout << "@@@--------------------------------@@@\n";
	std::cout << "rvalue: " << derived << '\n';
	std::cout << "@@@--------------------------------@@@\n";
}

Derived baz() {
	return Derived("derived-local");
}

// compile with -fno-elide-constructors to see the moving.

int main() {

	int choice = 5;

	
	if (choice == 0) {  // left value
		Derived d0("derived_0");
		baz_value(d0);
	} else if (choice == 1) {  // right value
		baz_value(Derived("derived_1"));
	} else if (choice == 2) {  // right value returned from function
		baz_value(baz());
	} else if (choice == 3) {
		Derived d0("derived_0");
		Derived &lderived = d0;
		baz_value(lderived);
	} else if (choice == 4) {
		Derived &&rderived = baz();
		baz_value(rderived);
	} else if (choice == 5) {  // Postfix return rvalue
		Derived d0("derived_0");
		baz_value(d0++);
	} else if (choice == 6) {  // Prefix return lvalue
		Derived d0("derived_0");
		baz_value(++d0);
	} else if (choice == 7) {
		Derived d0("derived_0");
		Derived *pD1 = &++d0;
		// Derived *pD1 = &d0++;//error: taking address of rvalue
		baz_value(*pD1);
	} else if (choice == 8) {  // Similar to postfix return rvalue
		baz_value((*foo())++);
	} else if (choice == 9) {  // Similar to pretfix return lvalue
		baz_value(++(*foo()));
	} else if (choice == 10) {
		Derived d0("derived_0");
		std::cout << "\n";
		Derived d1("derived_1");
		std::cout << "\n";
		Derived d2("derived_2");
		std::cout << "\n";
		d1 = std::move(d0);
		std::cout << "\n";
		d2 = d0;
		std::cout << "\n";
	}

	std::cout << "\n";

	return 0;

}
