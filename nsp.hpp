/*
 *  nsp.hpp - A constexpr fancy pointer for C++20 providing automatic null initialization/checks.
 *
 *  Copyright (c) 2026 3T5K
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 *  FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
 *  OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 *  OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef LIB_NSP_HPP
#define LIB_NSP_HPP

#include <exception>
#include <type_traits>
#include <concepts>
#include <memory>
#include <compare>
#include <functional>

namespace nsp
{

struct NullPtrDeref : std::exception
{
    [[nodiscard]] auto what() const noexcept -> const char * override
    {
        return "null pointer dereference attempted";
    }
};

template <typename From, typename To>
concept PtrConvTo = std::convertible_to< std::add_pointer_t<From>
                                       , std::add_pointer_t<To>>;

template <template <typename> class Derived, typename ElemType>
struct Deref
{
    [[nodiscard]] constexpr auto operator->() const -> ElemType *
    {
        ElemType *const ptr = static_cast<const Derived<ElemType> &>(*this).ptr;
        if (ptr == nullptr) {
            throw NullPtrDeref{};
        }

        return ptr;
    }

    [[nodiscard]] constexpr auto operator*() const -> ElemType &
    {
        return *this->operator->();
    }

    [[nodiscard]] constexpr auto value() const -> ElemType &
    {
        return this->operator*();
    }
};

template <template <typename> class Derived, typename ElemType>
    requires std::is_void_v<ElemType>
struct Deref<Derived, ElemType> { };

template <template <typename> class Derived, typename ElemType>
struct PointerTo
{
    [[nodiscard]] static constexpr auto pointer_to(ElemType &v) noexcept -> Derived<ElemType>
    {
        return std::addressof(v);
    }
};

template <template <typename> class Derived, typename VoidTp>
    requires std::is_void_v<VoidTp>
struct PointerTo<Derived, VoidTp>
{
    [[nodiscard]] static constexpr auto pointer_to(PtrConvTo<VoidTp> auto &v) noexcept -> Derived<VoidTp>
    {
        return std::addressof(v);
    }
};

/*
 * Maybe these should be non-member overloads as is customary in the stdlib.
 * Could result in better errors.
 */
template <template <typename> class Derived, typename ElemType>
struct Cmp3Way
{
    [[nodiscard]] constexpr auto operator<=>(const Derived<ElemType> &oth) const noexcept
        -> std::compare_three_way_result_t<ElemType *>
    {
        return std::compare_three_way{}(
            static_cast<const Derived<ElemType> &>(*this).ptr, oth.ptr);
    }

    [[nodiscard]] constexpr auto operator<=>(std::nullptr_t) const noexcept
        -> std::compare_three_way_result_t<ElemType *>
    {
        return std::compare_three_way{}(
            static_cast<const Derived<ElemType> &>(*this).ptr, nullptr);
    }
};

template <template <typename> class Derived, typename ElemType>
    requires (!std::three_way_comparable<ElemType *>)
struct Cmp3Way<Derived, ElemType> { };

template <typename T>
    requires (!std::is_reference_v<T>)
struct NullSafePtr : Deref<NullSafePtr, T>, PointerTo<NullSafePtr, T>, Cmp3Way<NullSafePtr, T>
{
    using element_type    = T;
    using pointer         = NullSafePtr;
    using raw_pointer     = element_type *;
    using difference_type = std::ptrdiff_t;

    template <typename U>
    using rebind = NullSafePtr<U>;

    raw_pointer ptr;

    [[nodiscard]] constexpr NullSafePtr(const raw_pointer p) noexcept
        : ptr{p}
    { }

    [[nodiscard]] constexpr NullSafePtr(std::nullptr_t) noexcept
        : ptr{nullptr}
    { }

    [[nodiscard]] constexpr NullSafePtr() noexcept
        : ptr{nullptr}
    { }

    template <PtrConvTo<element_type> U>
    [[nodiscard]] constexpr NullSafePtr(const NullSafePtr<U> nsp) noexcept
        : ptr{nsp.ptr}
    { }

    [[nodiscard]] constexpr explicit operator raw_pointer() const noexcept
    {
        return ptr;
    }

    [[nodiscard]] constexpr explicit operator bool() const noexcept
    {
        return static_cast<bool>(ptr);
    }

    template <typename... Args>
        requires std::invocable<element_type, Args...>
    [[nodiscard]] constexpr auto operator()(Args&&... args) const
        -> std::invoke_result_t<element_type, Args...>
    {
        return std::invoke(this->operator->(), std::forward<Args>(args)...);
    }

    [[nodiscard]] constexpr auto has_value() const noexcept
    {
        return ptr != nullptr;
    }

    [[nodiscard]] constexpr auto is_null() const noexcept
    {
        return ptr == nullptr;
    }

    [[nodiscard]] constexpr auto as_const() const noexcept -> NullSafePtr<std::add_const_t<element_type>>
        requires (!std::is_const_v<element_type>)
    {
        return ptr;
    }

    [[nodiscard]] constexpr auto operator==(const NullSafePtr &oth) const noexcept -> bool
    {
        return ptr == oth.ptr;
    };

    [[nodiscard]] constexpr auto operator==(std::nullptr_t) const noexcept -> bool
    {
        return ptr == nullptr;
    }

    [[nodiscard]] static auto to_address(const NullSafePtr p) noexcept -> raw_pointer
    {
        return p.ptr;
    }
};

} // namespace nsp

template <typename T>
struct std::pointer_traits<nsp::NullSafePtr<T>> : nsp::PointerTo<nsp::NullSafePtr, T>
{
    using pointer         = nsp::NullSafePtr<T>;
    using element_type    = typename pointer::element_type;
    using difference_type = typename pointer::difference_type;
    
    template <typename U>
    using rebind = typename pointer::template rebind<U>;

    [[nodiscard]] static constexpr auto to_address(const pointer p) noexcept -> element_type *
    {
        return pointer::to_address(p);
    }
};

#endif // LIB_NSP_HPP
