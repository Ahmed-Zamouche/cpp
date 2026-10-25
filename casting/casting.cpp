#include <array>
#include <atomic>
#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

#ifndef NDEBUG
    #define LOG_EVENT(msg) std::cout << msg << '\n'
#else
    #define LOG_EVENT(msg) do {} while (false)
#endif

class Object {
private:
    static inline std::atomic<int> s_ids{0};

protected:
    int id = s_ids++;
    std::string name{"noname"};

public:
    explicit Object(std::string name) : name(std::move(name)) {
        LOG_EVENT("Object(std::string &) `" << to_string() << "` created");
    }

    Object() {
        LOG_EVENT("Object() `" << to_string() << "` created");
    }

    virtual ~Object() {
        LOG_EVENT("Object `" << to_string() << "` destroyed");
    }

    Object(const Object& o) : id(s_ids++), name(o.name) {
        LOG_EVENT("Object `" << to_string() << "` copied");
    }

    Object& operator=(const Object& o) {
        if (this != &o) {
            name = o.name;
            LOG_EVENT("Object `" << to_string() << "` copied");
        }
        return *this;
    }

    Object(Object&& o) noexcept : id(s_ids++), name(std::move(o.name)) {
        LOG_EVENT("Object `" << to_string() << "` moved");
    }

    Object& operator=(Object&& o) noexcept {
        if (this != &o) {
            name = std::move(o.name);
            LOG_EVENT("Object `" << to_string() << "` moved");
        }
        return *this;
    }

    bool operator<(const Object& o) const noexcept {
        return id < o.id;
    }

    virtual std::string to_string() const {
        std::ostringstream ss;
        ss << "Object: {id: " << id << ", name: " << name << '}';
        return ss.str();
    }
};

inline std::ostream& operator<<(std::ostream& os, const Object& obj) {
    return os << obj.to_string();
}

class Base : public Object {
public:
    static constexpr std::size_t SIZE = 32;

private:
    std::unique_ptr<std::array<int, SIZE>> buffer;

public:
    explicit Base(std::string name)
        : Object(std::move(name)),
          buffer(std::make_unique<std::array<int, SIZE>>()) {
        buffer->fill(0);
        LOG_EVENT("Base(std::string &) `" << to_string() << "` created");
    }

    Base()
        : buffer(std::make_unique<std::array<int, SIZE>>()) {
        buffer->fill(0);
        LOG_EVENT("Base() `" << to_string() << "` created");
    }

    ~Base() override {
        LOG_EVENT("Base `" << to_string() << "` destroyed");
    }

    Base(const Base& o)
        : Object(o),
          buffer(o.buffer ? std::make_unique<std::array<int, SIZE>>(*o.buffer) : nullptr) {
        LOG_EVENT("Base `" << to_string() << "` copied");
    }

    Base& operator=(const Base& o) {
        if (this != &o) {
            Object::operator=(o);
            if (o.buffer) {
                buffer = std::make_unique<std::array<int, SIZE>>(*o.buffer);
            } else {
                buffer.reset();
            }
            LOG_EVENT("Base `" << to_string() << "` copied");
        }
        return *this;
    }

    Base(Base&& o) noexcept = default;
    Base& operator=(Base&& o) noexcept = default;

    std::string to_string() const override {
        std::ostringstream ss;
        ss << "Base: {" << Object::to_string() << ", buffer: " << static_cast<const void*>(buffer.get()) << "}";
        return ss.str();
    }
};

template <class T>
class Derived : public Base {
private:
    T value{};

public:
    explicit Derived(std::string name) : Base(std::move(name)) {
        LOG_EVENT("Derived(std::string &) `" << to_string() << "` created");
    }

    Derived() {
        LOG_EVENT("Derived() `" << to_string() << "` created");
    }

    ~Derived() override {
        LOG_EVENT("Derived `" << to_string() << "` destroyed");
    }

    Derived(const Derived&) = default;
    Derived& operator=(const Derived&) = default;
    Derived(Derived&&) noexcept = default;
    Derived& operator=(Derived&&) noexcept = default;

    Derived operator++(int) { // Postfix
        Derived r = *this;
        ++(*this);
#ifndef NDEBUG
        std::cout << "    Derived++ `" << to_string() << "`\n";
#endif
        return r;
    }

    Derived& operator++() { // Prefix
        ++value;
#ifndef NDEBUG
        std::cout << "    ++Derived `" << to_string() << "`\n";
#endif
        return *this;
    }

    std::string to_string() const override {
        std::ostringstream ss;
        ss << "Derived: {" << Base::to_string() << ", value: " << value << "}";
        return ss.str();
    }
};

int main() {
    Base b("base");

    Derived<int> dInt0("derived_int");
    Derived<float> dFloat0("derived_float");

    std::cout << '\n';
    ++dInt0;
    ++dInt0;
    ++dFloat0;
    std::cout << '\n';

    // dangerous: static_cast downcasts without type validation
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

    // safe alternative: dynamic_cast validates types at runtime and returns nullptr on failure
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

    // dangerous: use static_cast or dynamic_cast first
    std::cout << "reinterpret_cast: C style cast" << '\n';

    // Assign rvalue reference to Base
    Base &&d0 = Derived<int>("derived_int_0");
    Base &&d1 = static_cast<Base&&>(dFloat0);

    std::cout << '\n';
    std::cout << d0 << '\n';
    std::cout << d1 << '\n';
    std::cout << '\n';

    std::cout << "--------\n";
    return 0;
}
