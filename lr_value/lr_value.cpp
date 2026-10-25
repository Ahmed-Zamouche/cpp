#include <array>
#include <atomic>
#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#ifndef NDEBUG
    #define LOG_EVENT(msg) std::cout << msg << '\n'
#else
    #define LOG_EVENT(msg) do {} while (false)
#endif

// =============================================================================
// Class Hierarchy (Original Names Kept)
// =============================================================================

class Object {
private:
    static inline std::atomic<int> s_ids{0};

protected:
    int id = s_ids++;
    std::string name{"noname"};

public:
    explicit Object(std::string name) : name(std::move(name)) {
        LOG_EVENT("Object(std::string) `" << to_string() << "` created");
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
            LOG_EVENT("Object `" << to_string() << "` copy-assigned");
        }
        return *this;
    }

    Object(Object&& o) noexcept : id(s_ids++), name(std::move(o.name)) {
        LOG_EVENT("Object `" << to_string() << "` moved");
    }

    Object& operator=(Object&& o) noexcept {
        if (this != &o) {
            name = std::move(o.name);
            LOG_EVENT("Object `" << to_string() << "` move-assigned");
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
        LOG_EVENT("Base(std::string) `" << to_string() << "` created");
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
            LOG_EVENT("Base `" << to_string() << "` copy-assigned");
        }
        return *this;
    }

    Base(Base&& o) noexcept = default;
    Base& operator=(Base&& o) noexcept = default;

    std::string to_string() const override {
        std::ostringstream ss;
        ss << "Base: {" << Object::to_string() 
           << ", buffer: " << static_cast<const void*>(buffer.get()) << "}";
        return ss.str();
    }
};

class Derived : public Base {
private:
    int value{0};

public:
    explicit Derived(std::string name) : Base(std::move(name)) {
        LOG_EVENT("Derived(std::string) `" << to_string() << "` created");
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

    Derived operator++(int) {
        Derived copy = *this;
        ++(*this);
        LOG_EVENT("    Derived++ `" << to_string() << "`");
        return copy;
    }

    Derived& operator++() {
        ++value;
        LOG_EVENT("    ++Derived `" << to_string() << "`");
        return *this;
    }

    std::string to_string() const override {
        std::ostringstream ss;
        ss << "Derived: {" << Base::to_string() << ", value: " << value << "}";
        return ss.str();
    }
};

// =============================================================================
// Helper Functions for Inspection and Forwarding
// =============================================================================

// Target Overload 1: Lvalue reference
void inspect_value(const Derived& obj) {
    std::cout << "[LVALUE overload called] -> " << obj << '\n';
}

// Target Overload 2: Rvalue reference
void inspect_value(Derived&& obj) {
    std::cout << "[RVALUE overload called] -> " << obj << '\n';
}

// Factory returning prvalue
Derived create_derived(std::string name = "temp_derived") {
    return Derived(std::move(name));
}

// Factory returning heap object via unique_ptr
std::unique_ptr<Derived> make_heap_derived(std::string name = "heap_derived") {
    return std::make_unique<Derived>(std::move(name));
}

// -----------------------------------------------------------------------------
// Forwarding Demonstrations (T&& = Universal / Forwarding Reference)
// -----------------------------------------------------------------------------

// Flawed: 'arg' has a name, so it evaluates as an lvalue inside the function body
template <typename T>
void relay_unconditionally_as_lvalue(T&& arg) {
    std::cout << "--> relay_unconditionally_as_lvalue: passes 'arg' directly\n";
    inspect_value(arg); // Always calls inspect_value(const Derived&)
}

// Flawed: unconditionally casts to rvalue, which strips lvalueness from lvalues
template <typename T>
void relay_unconditionally_as_rvalue(T&& arg) {
    std::cout << "--> relay_unconditionally_as_rvalue: passes std::move(arg)\n";
    inspect_value(std::move(arg)); // Always calls inspect_value(Derived&&)
}

// Correct: preserves the caller's value category using std::forward
template <typename T>
void relay_perfectly(T&& arg) {
    std::cout << "--> relay_perfectly: passes std::forward<T>(arg)\n";
    inspect_value(std::forward<T>(arg));
}

// =============================================================================
// Learning Suite (main)
// =============================================================================

int main() {
    // Change this value (0 to 13) to test individual concepts:
    int choice = 11;

    // 0: Named variable is an Lvalue
    if (choice == 0) {
        Derived d0("derived_0");
        inspect_value(d0); // Calls lvalue overload
    }

    // 1: Temporary object is a Prvalue
    else if (choice == 1) {
        inspect_value(Derived("derived_1")); // Calls rvalue overload
    }

    // 2: Return-by-value function produces a Prvalue (guaranteed elision in C++17)
    else if (choice == 2) {
        inspect_value(create_derived()); // Calls rvalue overload
    }

    // 3: Lvalue references are Lvalues
    else if (choice == 3) {
        Derived d0("derived_0");
        Derived& lref = d0;
        inspect_value(lref); // Calls lvalue overload
    }

    // 4: Named Rvalue Reference evaluates as an Lvalue inside expressions!
    else if (choice == 4) {
        Derived&& rref = create_derived();
        inspect_value(rref); // Calls LVALUE overload because 'rref' has a name
    }

    // 5: Postfix operator++ returns a temporary copy by value (Prvalue)
    else if (choice == 5) {
        Derived d0("derived_0");
        inspect_value(d0++); // Calls rvalue overload
    }

    // 6: Prefix operator++ returns *this by reference (Lvalue)
    else if (choice == 6) {
        Derived d0("derived_0");
        inspect_value(++d0); // Calls lvalue overload
    }

    // 7: Address-of (&) requires an Lvalue
    else if (choice == 7) {
        Derived d0("derived_0");
        Derived* ptr = &++d0; // Valid: ++d0 is an lvalue
        // Derived* err = &(d0++); // COMPILE ERROR: cannot take address of prvalue
        inspect_value(*ptr);
    }

    // 8: Pointer Dereference + Postfix returns a temporary copy (Prvalue)
    else if (choice == 8) {
        inspect_value((*make_heap_derived())++); // Calls rvalue overload
    }

    // 9: Pointer Dereference + Prefix returns a reference (Lvalue)
    else if (choice == 9) {
        inspect_value(++(*make_heap_derived())); // Calls lvalue overload
    }

    // 10: std::move vs. Copy Assignment & The Moved-From State
    else if (choice == 10) {
        Derived d0("derived_0");
        Derived d1("derived_1");
        Derived d2("derived_2");

        std::cout << "\n-- Move Assignment (d1 = std::move(d0)) --\n";
        d1 = std::move(d0);
        std::cout << "d0 (moved-from): " << d0 << '\n';
        std::cout << "d1 (new owner):  " << d1 << '\n';

        std::cout << "\n-- Copy Assignment from moved-from object (d2 = d0) --\n";
        d2 = d0;
        std::cout << "d2:              " << d2 << '\n';
    }

    // 11: Reference Collapsing in Explicit Type Aliases
    // Rules:
    //   &  + &  -> &
    //   &  + && -> &
    //   && + &  -> &
    //   && + && -> &&
    else if (choice == 11) {
        Derived d("collapsing_demo");

        using LRef = Derived&;
        using RRef = Derived&&;

        // Rule 1: (& + & -> &)
        using Rule1 = LRef&;
        static_assert(std::is_same_v<Rule1, Derived&>, "Must collapse to Derived&");
        Rule1 r1 = d;
        std::cout << "Rule 1 (& + & -> &):   ";
        inspect_value(r1);

        // Rule 2: (& + && -> &)
        using Rule2 = LRef&&;
        static_assert(std::is_same_v<Rule2, Derived&>, "Must collapse to Derived&");
        Rule2 r2 = d;
        std::cout << "Rule 2 (& + && -> &):  ";
        inspect_value(r2);

        // Rule 3: (&& + & -> &)
        using Rule3 = RRef&;
        static_assert(std::is_same_v<Rule3, Derived&>, "Must collapse to Derived&");
        Rule3 r3 = d;
        std::cout << "Rule 3 (&& + & -> &):  ";
        inspect_value(r3);

        // Rule 4: (&& + && -> &&)
        using Rule4 = RRef&&;
        static_assert(std::is_same_v<Rule4, Derived&&>, "Must collapse to Derived&&");
        Rule4 r4 = create_derived();
        std::cout << "Rule 4 (&& + && -> &&): ";
        inspect_value(std::move(r4));
    }

    // 12: Why We Need std::forward (The Forwarding Problem)
    else if (choice == 12) {
        Derived d("original_d");

        std::cout << "--- 1. Passing temporary to relay_unconditionally_as_lvalue ---\n";
        // Loss of rvalueness: passed rvalue, but wrapper calls lvalue overload
        relay_unconditionally_as_lvalue(create_derived());

        std::cout << "\n--- 2. Passing lvalue to relay_unconditionally_as_rvalue ---\n";
        // Dangerous move: passed lvalue, but wrapper forces it into rvalue overload
        relay_unconditionally_as_rvalue(d);
    }

    // 13: Perfect Forwarding with std::forward<T>
    else if (choice == 13) {
        Derived d("original_d");

        std::cout << "--- Forwarding an Lvalue ---\n";
        // T deduces to Derived& -> T&& collapses to Derived& -> forward yields Derived&
        relay_perfectly(d);

        std::cout << "\n--- Forwarding an Rvalue ---\n";
        // T deduces to Derived -> T&& is Derived&& -> forward yields Derived&&
        relay_perfectly(create_derived());
    }

    std::cout << '\n';
    return 0;
}
