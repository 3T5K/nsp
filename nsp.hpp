/*
 *  nsp.hpp - A constexpr fancy pointer providing automatic null initialization/checks.
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
    using derived_reference = std::add_lvalue_reference_t<std::add_const_t<Derived<ElemType>>>;

    [[nodiscard]] constexpr auto operator->() const -> ElemType *
    {
        ElemType *const ptr = static_cast<derived_reference>(*this).ptr;
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

template <typename T>
    requires (!std::is_reference_v<T>)
struct NullSafePtr : Deref<NullSafePtr, T>, PointerTo<NullSafePtr, T>
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

    [[nodiscard]] constexpr auto has_value() const noexcept
    {
        return ptr != nullptr;
    }

    [[nodiscard]] constexpr auto is_null() const noexcept
    {
        return ptr == nullptr;
    }

    [[nodiscard]] constexpr auto as_const() const noexcept
        -> NullSafePtr<std::add_const_t<element_type>>
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

    [[nodiscard]] constexpr auto operator<=>(const NullSafePtr &oth) const noexcept
        -> std::compare_three_way_result_t<raw_pointer>
    {
        return std::compare_three_way(ptr, oth.ptr);
    }

    [[nodiscard]] constexpr auto operator<=>(std::nullptr_t) const noexcept
        -> std::compare_three_way_result_t<raw_pointer>
    {
        return std::compare_three_way(ptr, nullptr);
    }

    [[nodiscard]] static auto to_address(const NullSafePtr p) noexcept -> raw_pointer
    {
        return p.ptr;
    }
};

} // namespace nsp

template <typename T>
struct std::pointer_traits<nsp::NullSafePtr<T>>
{
    using pointer         = nsp::NullSafePtr<T>;
    using element_type    = typename pointer::element_type;
    using difference_type = typename pointer::difference_type;
    
    template <typename U>
    using rebind = typename pointer::template rebind<U>;

    [[nodiscard]] static constexpr auto pointer_to(element_type &r) noexcept -> pointer
    {
        return pointer::pointer_to(r);
    }

    [[nodiscard]] static constexpr auto to_address(const pointer p) noexcept -> pointer
    {
        return pointer::to_address(p);
    }
};

#endif // LIB_NSP_HPP
