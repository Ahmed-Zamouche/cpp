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
        LOG_EVENT("Object(std::string) `" << to_string() << "` created");
    }

    Object() {
        LOG_EVENT("Object() `" << to_string() << "` created");
    }

    virtual ~Object() {
        LOG_EVENT("Object `" << to_string() << "` destroyed");
    }

    // Preserve unique ID identity upon copy/move
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
    // C++17 RAII buffer replaces raw new[] / delete[]
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
        ss << "Base: {" << Object::to_string() << ", buffer: " << static_cast<const void*>(buffer.get()) << "}";
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

    // Rule of Zero: let compiler manage members and Base subobjects cleanly
    Derived(const Derived&) = default;
    Derived& operator=(const Derived&) = default;
    Derived(Derived&&) noexcept = default;
    Derived& operator=(Derived&&) noexcept = default;

    // Postfix ++: returns copy by value (prvalue)
    Derived operator++(int) {
        Derived r = *this;
        ++(*this);
        LOG_EVENT("    Derived++ `" << to_string() << "`");
        return r;
    }

    // Prefix ++: returns reference to self (lvalue)
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

// Returns a unique_ptr by value (uses move semantics / NRVO)
std::unique_ptr<Derived> foo() {
    return std::make_unique<Derived>("unique_ptr");
}

// Returns an lvalue reference to static storage
int& bar() {
    static int s_int = 0;
    return s_int;
}

// Overload A: binds to const lvalues (and rvalues if no rvalue overload is available)
void baz_value(const Derived &derived) {
    std::cout << "@@@--------------------------------@@@\n";
    std::cout << "lvalue overload called: " << derived << '\n';
    std::cout << "@@@--------------------------------@@@\n";
}

// Overload B: preferred exact match for rvalues (prvalues and xvalues)
void baz_value(Derived &&derived) {
    std::cout << "@@@--------------------------------@@@\n";
    std::cout << "rvalue overload called: " << derived << '\n';
    std::cout << "@@@--------------------------------@@@\n";
}

// Factory function returning a temporary Derived object by value
Derived baz() {
    return Derived("derived-local");
}


int main() {
    // Change this value from 0 to 10 to test each specific concept:
    int choice = 5;

    // =========================================================================
    // CHOICE 0: Named Objects are Lvalues
    // Lesson:
    // - 'd0' has a name, a storage address, and persists across multiple expressions.
    // - Overload resolution selects 'baz_value(const Derived&)' because an lvalue
    //   cannot bind to a non-const rvalue reference (Derived&&).
    // =========================================================================
    if (choice == 0) {
        Derived d0("derived_0");
        baz_value(d0); // Selects lvalue overload
    }

    // =========================================================================
    // CHOICE 1: Temporary Objects are Prvalues
    // Lesson:
    // - Derived("derived_1") creates an unnamed temporary (prvalue).
    // - Overload resolution chooses 'baz_value(Derived&&)' because an rvalue 
    //   reference is a more specific match than 'const Derived&'.
    // - The temporary is destroyed immediately after the statement finishes.
    // =========================================================================
    else if (choice == 1) {
        baz_value(Derived("derived_1")); // Selects rvalue overload
    }

    // =========================================================================
    // CHOICE 2: Return-by-Value Functions Yield Prvalues (C++17 Guaranteed Copy Elision)
    // Lesson:
    // - baz() returns a Derived object by value.
    // - In C++17, prvalue return initialization is guaranteed to construct directly
    //   in place without extra temporary copies or moves.
    // - The return expression is still an rvalue, so 'baz_value(Derived&&)' is invoked.
    // =========================================================================
    else if (choice == 2) {
        baz_value(baz()); // Selects rvalue overload
    }

    // =========================================================================
    // CHOICE 3: Lvalue References are Themselves Lvalues
    // Lesson:
    // - 'lderived' is an alias (Derived&) to an existing lvalue ('d0').
    // - Evaluating 'lderived' gives an lvalue expression.
    // - Calls 'baz_value(const Derived&)'.
    // =========================================================================
    else if (choice == 3) {
        Derived d0("derived_0");
        Derived &lderived = d0;
        baz_value(lderived); // Selects lvalue overload
    }

    // =========================================================================
    // CHOICE 4: "Named Rvalue References are Lvalues Inside Expressions"
    // Lesson (Crucial Modern C++ Gotcha):
    // - 'rderived' binds to an rvalue via lifetime extension (the temporary from baz()
    //   lives as long as 'rderived' is in scope).
    // - HOWEVER, because 'rderived' HAS A NAME, using 'rderived' as an expression
    //   evaluates to an LVALUE!
    // - Therefore, 'baz_value(rderived)' calls the LVALUE overload!
    // - To invoke the rvalue overload here, you would have to write std::move(rderived).
    // =========================================================================
    else if (choice == 4) {
        Derived &&rderived = baz();
        baz_value(rderived); // Selects LVALUE overload despite type being 'Derived&&'
    }

    // =========================================================================
    // CHOICE 5: Postfix operator++ Returns a Prvalue (By Value)
    // Lesson:
    // - Signature: Derived operator++(int). It makes a copy, increments original,
    //   and returns the unincremented copy BY VALUE.
    // - Because it is returned by value, the expression 'd0++' is a temporary (prvalue).
    // - Selects 'baz_value(Derived&&)'.
    // =========================================================================
    else if (choice == 5) {
        Derived d0("derived_0");
        baz_value(d0++); // Selects rvalue overload (passes the old temporary copy)
    }

    // =========================================================================
    // CHOICE 6: Prefix operator++ Returns an Lvalue Reference
    // Lesson:
    // - Signature: Derived& operator++(). It increments in place and returns *this.
    // - Returning a non-const reference makes '++d0' an LVALUE expression.
    // - Selects 'baz_value(const Derived&)'.
    // =========================================================================
    else if (choice == 6) {
        Derived d0("derived_0");
        baz_value(++d0); // Selects lvalue overload (passes mutated d0 directly)
    }

    // =========================================================================
    // CHOICE 7: Address-of Operator (&) Requires an Lvalue
    // Lesson:
    // - Because '++d0' is an lvalue, '&++d0' is completely valid and takes d0's address.
    // - Conversely, '&(d0++)' fails to compile because you CANNOT take the address
    //   of a prvalue temporary!
    // - Dereferencing '*pD1' yields an lvalue, calling 'baz_value(const Derived&)'.
    // =========================================================================
    else if (choice == 7) {
        Derived d0("derived_0");
        Derived *pD1 = &++d0;  // Legal: ++d0 is an lvalue
        // Derived *pD1 = &(d0++); // COMPILE ERROR: lvalue required as unary '&' operand
        baz_value(*pD1);
    }

    // =========================================================================
    // CHOICE 8: Pointer Dereference + Postfix Operator
    // Lesson:
    // - foo() returns a std::unique_ptr<Derived> by value.
    // - '*foo()' yields an lvalue reference to the heap-allocated Derived.
    // - Calling '(*foo())++' executes postfix increment on the heap object,
    //   returning a temporary copy (prvalue).
    // - Selects 'baz_value(Derived&&)'.
    // - Both the temporary unique_ptr and the heap object are cleaned up at semicolon.
    // =========================================================================
    else if (choice == 8) {
        baz_value((*foo())++); // Selects rvalue overload
    }

    // =========================================================================
    // CHOICE 9: Pointer Dereference + Prefix Operator
    // Lesson:
    // - '++(*foo())' executes prefix increment and returns an lvalue reference
    //   to the Derived managed by the unique_ptr.
    // - Since the expression is an lvalue reference, it calls 'baz_value(const Derived&)'.
    // - Note: Once the full expression finishes, the temporary std::unique_ptr from foo()
    //   is destroyed, deleting the heap object.
    // =========================================================================
    else if (choice == 9) {
        baz_value(++(*foo())); // Selects lvalue overload
    }

    // =========================================================================
    // CHOICE 10: std::move vs Copy Assignment and the "Moved-From" State
    // Lesson:
    // 1. std::move(d0) does NOT move anything by itself; it casts d0 to an xvalue (Derived&&).
    // 2. 'd1 = std::move(d0);' invokes move-assignment:
    //    - d1 takes ownership of d0's dynamic resources (its buffer/name).
    //    - d0 is left in a valid, but "moved-from" state (empty/default strings, reset buffer).
    // 3. 'd2 = d0;' invokes copy-assignment:
    //    - Even though d0 was moved from, it remains a valid object, so it can still
    //      be safely copied! d2 gets whatever state d0 was left with.
    // =========================================================================
    else if (choice == 10) {
        Derived d0("derived_0");
        std::cout << "\n";
        Derived d1("derived_1");
        std::cout << "\n";
        Derived d2("derived_2");
        std::cout << "\n";

        std::cout << "-- Move Assignment (d1 = std::move(d0)) --\n";
        d1 = std::move(d0); // Casts d0 to xvalue -> invokes operator=(Derived&&)
        std::cout << "d0 after being moved: " << d0 << '\n';
        std::cout << "d1 after move target: " << d1 << '\n';
        std::cout << "\n";

        std::cout << "-- Copy Assignment from moved-from object (d2 = d0) --\n";
        d2 = d0; // d0 is valid; copy assignment operates normally
        std::cout << "d2 after copy:        " << d2 << '\n';
        std::cout << "\n";
    }

    std::cout << "\n";
    return 0;
}
