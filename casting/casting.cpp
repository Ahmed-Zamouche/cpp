#include <array>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <typeinfo>
#include <utility>

#ifndef NDEBUG
    #define LOG_EVENT(msg) std::cout << msg << '\n'
#else
    #define LOG_EVENT(msg) do {} while (false)
#endif

// =============================================================================
// Original Class Hierarchy (Untouched)
// =============================================================================

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

    void set_value(T new_value) { value = new_value; }
    T get_value() const { return value; }

    std::string to_string() const override {
        std::ostringstream ss;
        ss << "Derived: {" << Base::to_string() << ", value: " << value << "}";
        return ss.str();
    }
};

// =============================================================================
// Main Learning Suite: Casts in Modern C++
// =============================================================================

int main() {
    std::cout << "=========================================================\n";
    std::cout << "             C++ CAST MASTERCLASS CURRICULUM             \n";
    std::cout << "=========================================================\n\n";

    // -------------------------------------------------------------------------
    // LESSON 1: Upcasting (Implicit vs Explicit static_cast)
    // Concept:
    // - Derived "is-a" Base. Upcasting points to the base subobject.
    // - Always safe and can be done implicitly without any cast keyword.
    // -------------------------------------------------------------------------
    {
        std::cout << "--- LESSON 1: Upcasting (Derived* -> Base*) ---\n";
        Derived<int> derived_int("derived_int");

        // Implicit upcast (preferred, completely safe)
        Base* base_ptr_implicit = &derived_int;

        // Explicit static_cast upcast (valid, identical behavior)
        Base* base_ptr_explicit = static_cast<Base*>(&derived_int);

        // Virtual dispatch correctly resolves to Derived<int>::to_string()
        std::cout << "Polymorphic call via Base*: " << *base_ptr_implicit << '\n';
        assert(base_ptr_implicit == base_ptr_explicit);
        std::cout << '\n';
    }

    // -------------------------------------------------------------------------
    // LESSON 2: static_cast Downcasting Pitfalls (Compile-Time Blind Trust)
    // Concept:
    // - static_cast performs NO runtime checks. It trusts the programmer.
    // - Downcasting a Base* that doesn't actually point to the target Derived type
    //   compiles, but dereferencing causes UNDEFINED BEHAVIOR (UB).
    // -------------------------------------------------------------------------
    {
        std::cout << "--- LESSON 2: static_cast Downcasting (Blind Downcast) ---\n";
        Base standalone_base("standalone_base");
        Derived<int> actual_derived_int("actual_derived_int");

        // Valid downcast: pointer really points to Derived<int>
        Base* base_pointing_to_derived = &actual_derived_int;
        Derived<int>* safe_downcast = static_cast<Derived<int>*>(base_pointing_to_derived);
        std::cout << "Valid static downcast: " << safe_downcast->to_string() << '\n';

        // [DANGEROUS / UB]: 'standalone_base' is NOT a Derived<int>.
        // Compiles cleanly, but gives an invalid pointer to non-existent 'value' member.
        Derived<int>* invalid_derived_ptr = static_cast<Derived<int>*>(&standalone_base);
        std::cout << "DANGEROUS - Reading UB pointer: " << *invalid_derived_ptr << '\n';
        std::cout << '\n';
    }

    // -------------------------------------------------------------------------
    // LESSON 3: dynamic_cast with POINTERS (Safe RTTI Downcasting)
    // Concept:
    // - dynamic_cast uses Run-Time Type Information (RTTI) stored in the vtable.
    // - If cast fails on a POINTER, it safely returns nullptr.
    // -------------------------------------------------------------------------
    {
        std::cout << "--- LESSON 3: dynamic_cast with Pointers (nullptr on Failure) ---\n";
        Base standalone_base("standalone_base");
        Derived<int> actual_derived_int("actual_derived_int");

        Base* base_pointing_to_base = &standalone_base;
        Base* base_pointing_to_derived = &actual_derived_int;

        // Attempt 1: Fail expected (Base is not Derived<int>)
        Derived<int>* failed_cast = dynamic_cast<Derived<int>*>(base_pointing_to_base);
        if (failed_cast == nullptr) {
            std::cout << "Success: dynamic_cast safely returned nullptr for invalid cast.\n";
        }
        assert(failed_cast == nullptr);

        // Attempt 2: Success expected (points to dynamic type Derived<int>)
        Derived<int>* successful_cast = dynamic_cast<Derived<int>*>(base_pointing_to_derived);
        assert(successful_cast != nullptr);
        std::cout << "Success: dynamic_cast resolved valid pointer -> " << *successful_cast << '\n';
        std::cout << '\n';
    }

    // -------------------------------------------------------------------------
    // LESSON 4: dynamic_cast with REFERENCES (Throws std::bad_cast)
    // Concept:
    // - There is no such thing as a "null reference" in C++.
    // - When dynamic_cast fails on a reference, it throws std::bad_cast!
    // -------------------------------------------------------------------------
    {
        std::cout << "--- LESSON 4: dynamic_cast with References (std::bad_cast) ---\n";
        Base standalone_base("standalone_base");
        Base& base_ref = standalone_base;

        try {
            std::cout << "Attempting invalid dynamic_cast on reference...\n";
            // standalone_base is not Derived<int>, so this must throw!
            Derived<int>& derived_ref = dynamic_cast<Derived<int>&>(base_ref);
            // The following will never execute:
            std::cout << derived_ref << '\n';
        } catch (const std::bad_cast& e) {
            std::cout << "Caught expected std::bad_cast exception: " << e.what() << '\n';
        }
        std::cout << '\n';
    }

    // -------------------------------------------------------------------------
    // LESSON 5: const_cast (Removing constness: Safe vs Undefined Behavior)
    // Concept:
    // - const_cast can only add or remove const/volatile qualifiers.
    // - LEGAL: Modifying an originally NON-CONST object accessed via a const pointer/ref.
    // - UNDEFINED BEHAVIOR: Modifying an originally CONST object (e.g. stored in read-only memory).
    // -------------------------------------------------------------------------
    {
        std::cout << "--- LESSON 5: const_cast ---\n";

        // Case A: Safe modification of an originally non-const object
        Derived<int> mutable_object("mutable_object");
        mutable_object.set_value(10);

        const Derived<int>* read_only_view = &mutable_object;
        // read_only_view->set_value(20); // COMPILE ERROR: cannot mutate via const pointer

        Derived<int>* writable_view = const_cast<Derived<int>*>(read_only_view);
        writable_view->set_value(99); // Legal: underlying object was created non-const
        std::cout << "Value safely modified through const_cast: " << mutable_object.get_value() << '\n';

        // Case B: Demonstrating const_cast on a truly const object (Educational Warning)
        const Derived<int> truly_const_object("const_object");
        const Derived<int>* const_ptr = &truly_const_object;
        Derived<int>* dangerous_ptr = const_cast<Derived<int>*>(const_ptr);
        // dangerous_ptr->set_value(42); // DANGEROUS / UB: modifying an originally const object!
        (void)dangerous_ptr;
        std::cout << "Warning: Mutating truly_const_object via dangerous_ptr triggers Undefined Behavior!\n";
        std::cout << '\n';
    }

    // -------------------------------------------------------------------------
    // LESSON 6: Object Slicing vs Polymorphic References/Pointers
    // Concept:
    // - Passing or casting by VALUE invokes Base's copy constructor,
    //   SLICING OFF the Derived portion (the value member is lost, vtable resets to Base).
    // - Passing or casting by POINTER/REFERENCE retains the full derived object.
    // -------------------------------------------------------------------------
    {
        std::cout << "--- LESSON 6: Object Slicing (Value Cast vs Reference Cast) ---\n";
        Derived<int> full_derived("full_derived");
        full_derived.set_value(777);

        // SLICING OCCURS: Derived is copied into a value-type Base.
        // Derived<int>::value is permanently dropped!
        Base sliced_copy = static_cast<Base>(full_derived); // or: Base sliced_copy = full_derived;
        std::cout << "Sliced Base copy (value lost, vtable is Base): " << sliced_copy << '\n';

        // NO SLICING: Preserved via reference
        Base& preserved_reference = full_derived;
        std::cout << "Preserved Reference (polymorphism intact):     " << preserved_reference << '\n';
        std::cout << '\n';
    }

    // -------------------------------------------------------------------------
    // LESSON 7: reinterpret_cast (Raw Byte/Address Reinterpretation)
    // Concept:
    // - Reinterprets the raw bit pattern without conversion logic.
    // - Used for low-level programming (pointer-to-integer, inspecting memory bytes).
    // - Violates Strict Aliasing if used to access typed data arbitrarily.
    // -------------------------------------------------------------------------
    {
        std::cout << "--- LESSON 7: reinterpret_cast (Raw Bit / Pointer Reinterpretation) ---\n";
        Derived<int> item("item_bytes");

        // 1. Pointer to Integer Address
        auto raw_address = reinterpret_cast<std::uintptr_t>(&item);
        std::cout << "Object memory address as integer: 0x" << std::hex << raw_address << std::dec << '\n';

        // 2. Inspecting raw byte representation
        auto* byte_ptr = reinterpret_cast<unsigned char*>(&item);
        std::cout << "First 4 raw bytes in memory: ";
        for (std::size_t i = 0; i < 4; ++i) {
            std::cout << std::hex << static_cast<int>(byte_ptr[i]) << ' ';
        }
        std::cout << std::dec << "\n\n";
    }

    // -------------------------------------------------------------------------
    // LESSON 8: static_cast to Rvalue Reference (Base&&) & Lifetime Extension
    // Concept:
    // - static_cast<Base&&>(lvalue) turns an lvalue into an xvalue (enables moving).
    // - Binding a temporary to Base&& extends its lifetime to the reference's scope.
    // -------------------------------------------------------------------------
    {
        std::cout << "--- LESSON 8: static_cast to Rvalue Reference (&&) ---\n";

        // Lifetime extension of temporary Derived<int>
        Base&& temporary_extended = Derived<int>("temporary_derived");
        std::cout << "Temporary extended to scope of reference: " << temporary_extended << '\n';

        // Cast lvalue to xvalue for moving / rvalue-binding
        Derived<float> lvalue_float("lvalue_float");
        Base&& rvalue_ref_to_lvalue = static_cast<Base&&>(lvalue_float);
        std::cout << "Lvalue bound to rvalue ref via static_cast: " << rvalue_ref_to_lvalue << '\n';
        std::cout << '\n';
    }

    // -------------------------------------------------------------------------
    // LESSON 9: Cross-Casting (Unrelated Sibling Types)
    // Concept:
    // - static_cast CANNOT cast between unrelated sibling types Derived<int>* and Derived<float>*.
    //   The compiler rejects it.
    // - dynamic_cast checks the actual runtime type and returns nullptr safely.
    // -------------------------------------------------------------------------
    {
        std::cout << "--- LESSON 9: Cross-Casting Unrelated Types ---\n";
        Derived<int> derived_int("derived_int");
        Base* base_ptr = &derived_int;

        // static_cast:
        // Derived<float>* err = static_cast<Derived<float>*>(base_ptr); // Compiles blindly through Base*, but is UB!
        // Derived<float>* err2 = static_cast<Derived<float>*>(&derived_int); // COMPILE ERROR! Direct sibling cast disallowed.

        // dynamic_cast: Safely checks the RTTI and rejects the sibling cross-cast at runtime
        Derived<float>* cross_cast_result = dynamic_cast<Derived<float>*>(base_ptr);
        if (cross_cast_result == nullptr) {
            std::cout << "Success: dynamic_cast correctly identified that Derived<int> is NOT Derived<float>.\n";
        }
        assert(cross_cast_result == nullptr);
        std::cout << '\n';
    }

    std::cout << "=========================================================\n";
    std::cout << "                   ALL LESSONS COMPLETED                 \n";
    std::cout << "=========================================================\n";

    return 0;
}
