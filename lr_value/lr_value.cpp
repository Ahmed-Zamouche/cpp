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
    std::cout << "=========================================================\n";
    std::cout << "  MODERN C++ VALUE CATEGORIES & FORWARDING DEMO SUITE    \n";
    std::cout << "=========================================================\n\n";

    // -------------------------------------------------------------------------
    // Lesson 0: Named variable is an Lvalue
    // -------------------------------------------------------------------------
    {
        std::cout << "--- 0: Named Variable (Lvalue) ---\n";
        Derived d0("derived_0");
        inspect_value(d0); // Calls lvalue overload
        std::cout << '\n';
    }

    // -------------------------------------------------------------------------
    // Lesson 1: Temporary object is a Prvalue
    // -------------------------------------------------------------------------
    {
        std::cout << "--- 1: Temporary Object (Prvalue) ---\n";
        inspect_value(Derived("derived_1")); // Calls rvalue overload
        std::cout << '\n';
    }

    // -------------------------------------------------------------------------
    // Lesson 2: Return-by-value function produces a Prvalue
    // -------------------------------------------------------------------------
    {
        std::cout << "--- 2: Return-by-Value (Prvalue / C++17 Elision) ---\n";
        inspect_value(create_derived()); // Calls rvalue overload
        std::cout << '\n';
    }

    // -------------------------------------------------------------------------
    // Lesson 3: Lvalue references evaluate as Lvalues
    // -------------------------------------------------------------------------
    {
        std::cout << "--- 3: Lvalue Reference ---\n";
        Derived d0("derived_0");
        Derived& lref = d0;
        inspect_value(lref); // Calls lvalue overload
        std::cout << '\n';
    }

    // -------------------------------------------------------------------------
    // Lesson 4: Named Rvalue Reference evaluates as an Lvalue!
    // -------------------------------------------------------------------------
    {
        std::cout << "--- 4: Named Rvalue Reference evaluates as Lvalue ---\n";
        Derived&& rref = create_derived();
        inspect_value(rref); // Calls LVALUE overload because 'rref' has a name!
        std::cout << '\n';
    }

    // -------------------------------------------------------------------------
    // Lesson 5: Postfix operator++ returns a temporary copy by value (Prvalue)
    // -------------------------------------------------------------------------
    {
        std::cout << "--- 5: Postfix operator++ (Prvalue) ---\n";
        Derived d0("derived_0");
        inspect_value(d0++); // Calls rvalue overload
        std::cout << '\n';
    }

    // -------------------------------------------------------------------------
    // Lesson 6: Prefix operator++ returns *this by reference (Lvalue)
    // -------------------------------------------------------------------------
    {
        std::cout << "--- 6: Prefix operator++ (Lvalue Reference) ---\n";
        Derived d0("derived_0");
        inspect_value(++d0); // Calls lvalue overload
        std::cout << '\n';
    }

    // -------------------------------------------------------------------------
    // Lesson 7: Address-of operator (&) requires an Lvalue
    // -------------------------------------------------------------------------
    {
        std::cout << "--- 7: Address-of Operator (&) ---\n";
        Derived d0("derived_0");
        Derived* ptr = &++d0; // Valid: ++d0 is an lvalue
        // Derived* err = &(d0++); // COMPILE ERROR: cannot take address of prvalue
        inspect_value(*ptr);
        std::cout << '\n';
    }

    // -------------------------------------------------------------------------
    // Lesson 8: Dereferencing unique_ptr + Postfix yields a Prvalue
    // -------------------------------------------------------------------------
    {
        std::cout << "--- 8: Heap Dereference + Postfix (Prvalue) ---\n";
        inspect_value((*make_heap_derived())++); // Calls rvalue overload
        std::cout << '\n';
    }

    // -------------------------------------------------------------------------
    // Lesson 9: Dereferencing unique_ptr + Prefix yields an Lvalue
    // -------------------------------------------------------------------------
    {
        std::cout << "--- 9: Heap Dereference + Prefix (Lvalue) ---\n";
        inspect_value(++(*make_heap_derived())); // Calls lvalue overload
        std::cout << '\n';
    }

    // -------------------------------------------------------------------------
    // Lesson 10: std::move vs. Copy Assignment & The Moved-From State
    // -------------------------------------------------------------------------
    {
        std::cout << "--- 10: Move vs Copy Assignment & Moved-From State ---\n";
        Derived d0("derived_0");
        Derived d1("derived_1");
        Derived d2("derived_2");

        std::cout << "Move assigning d0 into d1:\n";
        d1 = std::move(d0);
        std::cout << "d0 (moved-from): " << d0 << '\n';
        std::cout << "d1 (new owner):  " << d1 << '\n';

        std::cout << "Copy assigning moved-from d0 into d2:\n";
        d2 = d0; // Safe: moved-from object is in a valid state
        std::cout << "d2:              " << d2 << '\n';
        std::cout << '\n';
    }

    // -------------------------------------------------------------------------
    // Lesson 11: Reference Collapsing rules in Type Aliases
    //   &  + &  -> &
    //   &  + && -> &
    //   && + &  -> &
    //   && + && -> &&
    // -------------------------------------------------------------------------
    {
        std::cout << "--- 11: Reference Collapsing ---\n";
        Derived d("collapsing_demo");

        using LRef = Derived&;
        using RRef = Derived&&;

        // Rule 1: & + & -> &
        using Rule1 = LRef&;
        static_assert(std::is_same_v<Rule1, Derived&>);
        Rule1 r1 = d;
        std::cout << "Rule 1 (& + & -> &):   ";
        inspect_value(r1);

        // Rule 2: & + && -> &
        using Rule2 = LRef&&;
        static_assert(std::is_same_v<Rule2, Derived&>);
        Rule2 r2 = d;
        std::cout << "Rule 2 (& + && -> &):  ";
        inspect_value(r2);

        // Rule 3: && + & -> &
        using Rule3 = RRef&;
        static_assert(std::is_same_v<Rule3, Derived&>);
        Rule3 r3 = d;
        std::cout << "Rule 3 (&& + & -> &):  ";
        inspect_value(r3);

        // Rule 4: && + && -> &&
        using Rule4 = RRef&&;
        static_assert(std::is_same_v<Rule4, Derived&&>);
        Rule4 r4 = create_derived();
        std::cout << "Rule 4 (&& + && -> &&): ";
        inspect_value(std::move(r4));
        std::cout << '\n';
    }

    // -------------------------------------------------------------------------
    // Lesson 12: Why std::forward is needed (Flawed Forwarding)
    // -------------------------------------------------------------------------
    {
        std::cout << "--- 12: The Forwarding Problem (Without std::forward) ---\n";
        Derived d("original_d");

        std::cout << "[Problem 1: Passed rvalue, but loses rvalueness]\n";
        relay_unconditionally_as_lvalue(create_derived());

        std::cout << "[Problem 2: Passed lvalue, but dangerously moved]\n";
        relay_unconditionally_as_rvalue(d);
        std::cout << '\n';
    }

    // -------------------------------------------------------------------------
    // Lesson 13: Perfect Forwarding with std::forward<T>
    // -------------------------------------------------------------------------
    {
        std::cout << "--- 13: Perfect Forwarding (std::forward<T>) ---\n";
        Derived d("original_d");

        std::cout << "Forwarding an Lvalue:\n";
        relay_perfectly(d); // Calls LVALUE overload

        std::cout << "Forwarding an Rvalue:\n";
        relay_perfectly(create_derived()); // Calls RVALUE overload
        std::cout << '\n';
    }

    std::cout << "=========================================================\n";
    std::cout << "                      DEMO COMPLETE                      \n";
    std::cout << "=========================================================\n";

    return 0;
}
