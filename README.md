# NSP

A single header null-safe pointer library for C++20.

**Features:**
 - `nullptr` default initialization
 - null-checked dereferencing (uses exceptions)
 - non-owning semantics
 - same size as a raw pointer
 - full constexpr support
 - liberal use of `[[nodiscard]]`
 - `noexcept` everywhere it should be
 - `std::pointer_traits`, including `to_address`
 - equality comparison and ordering

**Does not support:**
 - pointer arithmetic
 - pointers to members

## Motivation

I wanted a type that can have an empty state or view an object with static storage duration.
Raw pointers are a decent abstraction for this, but dereferencing their "empty state" leads to UB.
With modern OSes, that means segfaults in practice, which are obviously not nice to deal with.

Then there is `std::optional<std::reference_wrapper<T>>`.
You get a checked accessor, default initialization to its empty state, and pointer-like assignment.
The problem is the space it takes up.
Implemenations of `std::optional<T>` are **usually** `alignof(T) + sizeof(T)` bytes.
A `std::reference_wrapper` is just a wrapper over a raw pointer.
So on systems with 64b pointers, that `optional` would take up `8 + 8` bytes.
That's 8 more bytes than a pointer for `optional`'s bookkeeping
(which is 1 byte in practice of which 1 bit is actually used)
and padding to preserve alignment.

## Demo

### Main Features
```cpp
nsp::NullSafePtr<int> p; // initialized with nullptr
int x = *p;              // throws nsp::NullPtrDeref
```

### Conversions

No constructors are explicit, so raw pointers convert implicitly.
```cpp
int x = 10;
nsp::NullSafePtr<int> p1 = &x; // copy-initialization allowed
```

Conversions from less qualified pointers to more qualified ones are supported.
```cpp
int x = 10;
nsp::NullSafePtr<int> p1{&x};
nsp::NullSafePtr<const int> p2{p1}; // ok
nsp::NullSafePtr<int> p3{p2};       // compile-time error
```

Additionally, `as_const` can be used on pointers to non-`const` to obtain pointers to `const`.
```cpp
int x = 10;
nsp::NullSafePtr<int> p1{&x};
auto p2 = p1.as_const();      // ok, produces nsp::NullSafePtr<const int>
auto p3 = p2.as_const();      // compile-time error, p2 is already a pointer to const
```

Conversions to equally or more qualified void pointers are allowed as well.
```cpp
int x = 10;
nsp::NullSafePtr<int> p1{&x};
nsp::NullSafePtr<void> p2{p1};       // ok
nsp::NullSafePtr<const void> p3{p1}; // ok
nsp::NullSafePtr<void> p4{&x};       // ok
```

Derived class pointer to base class pointer conversions work as expected too.
```cpp
Derived b;
nsp::NullSafePtr<Derived> p1{&b};
nsp::NullSafePtr<Base> p2{p1};        // ok
nsp::NullSafePtr<Base> p3{&b};        // ok
nsp::NullSafePtr<Derived> p4{p3};     // compile-time error
nsp::NullSafePtr<Derived> p5{p3.ptr}; // compile-time error
```

Direct conversion to a different non-void or non-base class pointer type (ignoring cv-qualification) is not allowed.
```cpp
int x = 10;
nsp::NullSafePtr<int> p1{&x};
nsp::NullSafePtr<float> p2{p1}; // compile-time error
nsp::NullSafePtr<void> p3{&x};
nsp::NullSafePtr<float> p4{p3}; // compile-time error
```

Implicit conversions to bool are disallowed.
```cpp
bool x = nsp::NullSafePtr<void>{}; // compile-time error
```

Conversions to the respective raw pointer type are explicit.
This is mainly to prevent accidental pointer arithmetic.
```cpp
nsp::NullSafePtr<int> p;
int *x = p + 10;                     // compile-time error
int *y = static_cast<int *>(p) + 10; // ok
```

### Void Pointers

A void pointer constitutes any specialization `nsp::NullSafePtr<T>` for which `std::is_void_v<std::pointer_traits<nsp::NullSafePtr<T>>::element_type>` is true.

Void pointers don't support:
  - `operator->()`
  - `operator*()`
  - `value()`

However, they do support:
  - `pointer_traits<...>::pointer_to()`
  - `pointer_traits<...>::to_address()`

```cpp
nsp::NullSafePtr<void> p1;
void *p2 = &(*p1)          // compile-time error
```

### Function Pointers

A function pointer constitutes any specialization `nsp::NullSafePtr<T>` for which `std::is_function_v<std::pointer_traits<nsp::NullSafePtr<T>>::element_type>` is true.

Function pointers don't support three way comparison.
Equality comparison is supported.
```cpp
nsp::NullSafePtr<void()> p1{};
nsp::NullSafePtr<void()> p2{};
bool x = p1 == p2;             // ok
auto y = p1 <=> p2;            // compile-time error
```

The call operator is also overloaded and uses `std::invoke` internally.
```cpp
nsp::NullSafePtr<int(int)> p1{&func};
nsp::NullSafePtr<int(int)> p2;
int x = p1(10);                       // ok
int y = p2(10);                       // throws nsp::NullPtrDeref
```

### Other Members

The relevant `std::pointer_traits<nsp::NullSafePtr<T>>` members are also exposed in `nsp::NullSafePtr<T>` itself.
 - `nsp::NullSafePtr<T>::element_type`
 - `nsp::NullSafePtr<T>::difference_type`
 - `nsp::NullSafePtr<T>::pointer`
 - `nsp::NullSafePtr<T>::rebind<U>`
 - `nsp::NullSafePtr<T>::to_address(...)`
 - `nsp::NullSafePtr<T>::pointer_to(...)`

Additional member types:
 - `nsp::NullSafePtr<T>::raw_pointer` => `element_type *`

Additional member functions:
 - `nsp::NullSafePtr<T>::is_null()`
 - `nsp::NullSafePtr<T>::has_value()`
 - `nsp::NullSafePtr<T>::value()` => invokes `operator*()`

Comparison functions:
 - `nsp::NullSafePtr<T>::operator==(const NullSafePtr &)`
 - `nsp::NullSafePtr<T>::operator==(std::nullptr_t)`
 - `nsp::NullSafePtr<T>::operator<=>(const NullSafePtr &)`
 - `nsp::NullSafePtr<T>::operator<=>(std::nullptr_t)`

## Installation

Install into `/usr/local/include`:
```sh
sudo make install
```

Uninstall:
```sh
sudo make uninstall
```
