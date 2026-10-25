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
    // -------------------------------------------------------------------------
    // LESSON 1: Object Construction, Hierarchy Initialization & Operator Overloading
    // -------------------------------------------------------------------------

    // Calls Object("base") -> Base("base"). Allocates internal buffer.
    Base b("base");

    // Instantiates class template for T = int and T = float.
    // Order of construction: Object -> Base -> Derived<T>.
    Derived<int> dInt0("derived_int");
    Derived<float> dFloat0("derived_float");

    std::cout << '\n';
    // Prefix increment (++value). Returns a reference (Derived<int>&) to the mutated instance.
    ++dInt0;   // dInt0.value becomes 1
    ++dInt0;   // dInt0.value becomes 2
    ++dFloat0; // dFloat0.value becomes 1.0f
    std::cout << '\n';

    // -------------------------------------------------------------------------
    // LESSON 2: static_cast Downcasting Pitfalls (Compile-Time Blind Trust)
    //
    // Learning Goal: Understand that static_cast performs NO runtime type checks.
    // Downcasting a Base pointer to a Derived pointer when the underlying object
    // is NOT actually that Derived type compiles fine, but causes UNDEFINED BEHAVIOR (UB)
    // the moment you dereference or access derived members.
    // -------------------------------------------------------------------------
    std::cout << "--- static_cast (Blind Downcasts) ---" << '\n';

    // [UB 1]: 'b' is purely a Base instance. It has no 'Derived<int>::value' member.
    // static_cast blindly assumes you know what you are doing.
    Derived<int> *pDi0 = static_cast<Derived<int>*>(&b);

    // [UB 2]: Exactly the same error, casting Base to Derived<float>.
    Derived<float> *pDf = static_cast<Derived<float>*>(&b);

    Base *pB;

    // Upcast: Safe and implicit. A Derived<int> is-a Base.
    pB = &dInt0;
    // [UB 3 - Cross-cast through base]: pB actually points to Derived<int>,
    // but static_cast forces it to become Derived<float>*.
    // Derived<int> and Derived<float> are completely unrelated types in memory!
    Derived<float> *pDif = static_cast<Derived<float>*>(pB);

    // Upcast: Pointing pB to Derived<float>.
    pB = &dFloat0;
    // [UB 4 - Cross-cast through base]: pB actually points to Derived<float>,
    // but static_cast forces it into Derived<int>*.
    Derived<int> *pDfi = static_cast<Derived<int>*>(pB);

    // Safe polymorphic call: pB points to dFloat0. Uses dynamic dispatch (vtable)
    // to invoke Derived<float>::to_string().
    std::cout << "*pB  : " << *pB << '\n';

    // DANGEROUS: Dereferencing pointers obtained via invalid static_casts.
    // If to_string() accesses 'value', it reads unallocated stack memory or
    // interprets raw bits of an unrelated type (int vs float bit representation).
    std::cout << "*pDi0: " << *pDi0 << '\n'; // Accesses 'value' on a raw Base object (UB)
    std::cout << "*pDf : " << *pDf << '\n';  // Accesses 'value' on a raw Base object (UB)
    std::cout << "*pDif: " << *pDif << '\n'; // Reads int bits as float (UB)
    std::cout << "*pDfi: " << *pDfi << '\n'; // Reads float bits as int (UB)
    std::cout << '\n';

    // -------------------------------------------------------------------------
    // LESSON 3: dynamic_cast (Runtime Type-Safe Downcasting & RTTI)
    //
    // Learning Goal: dynamic_cast queries the vtable (RTTI: Run-Time Type Information).
    // Unlike static_cast:
    // - If the target type does NOT match the real dynamic object, it returns nullptr (for pointers).
    // - Requires at least one virtual function in the base class.
    // -------------------------------------------------------------------------
    std::cout << "--- dynamic_cast (Safe RTTI Downcasts) ---" << '\n';

    Derived<int> *pDi1;

    // Safe Failure 1: 'b' is merely a Base, NOT a Derived<int>.
    // dynamic_cast detects this at runtime and safely returns nullptr.
    pDi1 = dynamic_cast<Derived<int>*>(&b);
    assert(pDi1 == nullptr);

    // Safe Failure 2: 'dFloat0' is Derived<float>, NOT Derived<int>.
    // dynamic_cast detects the type mismatch and safely returns nullptr.
    pDi1 = dynamic_cast<Derived<int>*>(&dFloat0);
    assert(pDi1 == nullptr);

    // Dynamic upcast (or exact match): b is Base, so casting to Base* succeeds.
    pB = dynamic_cast<Base*>(&b);

    // Safe Success: dInt0 IS dynamic type Derived<int>, so casting returns a valid pointer.
    pDi1 = dynamic_cast<Derived<int>*>(&dInt0);
    assert(pDi1 != nullptr);

    std::cout << '\n';
    std::cout << "*pB  : " << *pB << '\n';   // Prints Base: { ... }
    std::cout << "*pDi1: " << *pDi1 << '\n'; // Prints Derived: { ... value: 2 }
    std::cout << '\n';

    // -------------------------------------------------------------------------
    // LESSON 4: Rvalue References (&&) and Polymorphic Object Lifetime
    //
    // Learning Goal:
    // 1. Temporary object lifetime extension via reference binding.
    // 2. static_cast<Base&&> turns an lvalue into an xvalue (enabling moves/rvalue binding).
    // 3. Polymorphic behavior under rvalue references.
    // -------------------------------------------------------------------------
    std::cout << "--- Rvalue References (&&) & Lifetime Extension ---" << '\n';

    // A temporary Derived<int> is created on the right-hand side.
    // Binding it to a const reference (const Base&) or an rvalue reference (Base&&)
    // EXTENDS the lifetime of the temporary object to the lifetime of reference 'd0'.
    Base &&d0 = Derived<int>("derived_int_0");

    // 'dFloat0' is an existing named variable (an lvalue).
    // To bind an rvalue reference (Base&&) to an lvalue, you must cast it to an xvalue.
    // Note: This does NOT move anything by itself; it only treats dFloat0 as an rvalue reference.
    Base &&d1 = static_cast<Base&&>(dFloat0);

    std::cout << '\n';
    // Dynamic dispatch still works through references!
    // d0 invokes Derived<int>::to_string() via the virtual table.
    std::cout << "d0 (temporary): " << d0 << '\n';
    // d1 invokes Derived<float>::to_string() via the virtual table.
    std::cout << "d1 (cast lvalue): " << d1 << '\n';
    std::cout << '\n';

    std::cout << "--------\n";
    return 0;
    // Destruction order at scope exit:
    // 1. d0's temporary Derived<int> is destroyed (lifetime extension ends).
    // 2. dFloat0 is destroyed (Derived -> Base -> Object).
    // 3. dInt0 is destroyed.
    // 4. b is destroyed.
}
