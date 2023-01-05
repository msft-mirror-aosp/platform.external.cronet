//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// UNSUPPORTED: c++03, c++11, c++14, c++17

// template <class T>
//   constexpr int countl_zero(T x) noexcept;

// Constraints: T is an unsigned integer type
// Returns: The number of consecutive 0 bits, starting from the most significant bit.
//   [ Note: Returns N if x == 0. ]

#include <bit>
#include <cassert>
#include <cstdint>
#include <type_traits>

#include "test_macros.h"

struct A {};
enum       E1 : unsigned char { rEd };
enum class E2 : unsigned char { red };

template <class T>
constexpr bool test()
{
    ASSERT_SAME_TYPE(decltype(std::countl_zero(T())), int);
    ASSERT_NOEXCEPT(std::countl_zero(T()));
    T max = std::numeric_limits<T>::max();
    int dig = std::numeric_limits<T>::digits;

    assert(std::countl_zero(T(0)) == dig);
    assert(std::countl_zero(T(1)) == dig - 1);
    assert(std::countl_zero(T(2)) == dig - 2);
    assert(std::countl_zero(T(3)) == dig - 2);
    assert(std::countl_zero(T(4)) == dig - 3);
    assert(std::countl_zero(T(5)) == dig - 3);
    assert(std::countl_zero(T(6)) == dig - 3);
    assert(std::countl_zero(T(7)) == dig - 3);
    assert(std::countl_zero(T(8)) == dig - 4);
    assert(std::countl_zero(T(9)) == dig - 4);
    assert(std::countl_zero(T(127)) == dig - 7);
    assert(std::countl_zero(T(128)) == dig - 8);
    assert(std::countl_zero(max) == 0);

#ifndef TEST_HAS_NO_INT128
    if constexpr (std::is_same_v<T, __uint128_t>) {
        T val = T(128) << 32;
        assert(std::countl_zero(val-1) == 89);
        assert(std::countl_zero(val)   == 88);
        assert(std::countl_zero(val+1) == 88);
        val <<= 60;
        assert(std::countl_zero(val-1) == 29);
        assert(std::countl_zero(val)   == 28);
        assert(std::countl_zero(val+1) == 28);
    }
#endif

    return true;
}

int main(int, char**)
{
    {
    auto lambda = [](auto x) -> decltype(std::countl_zero(x)) {};
    using L = decltype(lambda);

    static_assert(!std::is_invocable_v<L, signed char>);
    static_assert(!std::is_invocable_v<L, short>);
    static_assert(!std::is_invocable_v<L, int>);
    static_assert(!std::is_invocable_v<L, long>);
    static_assert(!std::is_invocable_v<L, long long>);
#ifndef TEST_HAS_NO_INT128
    static_assert(!std::is_invocable_v<L, __int128_t>);
#endif

    static_assert(!std::is_invocable_v<L, int8_t>);
    static_assert(!std::is_invocable_v<L, int16_t>);
    static_assert(!std::is_invocable_v<L, int32_t>);
    static_assert(!std::is_invocable_v<L, int64_t>);
    static_assert(!std::is_invocable_v<L, intmax_t>);
    static_assert(!std::is_invocable_v<L, intptr_t>);
    static_assert(!std::is_invocable_v<L, ptrdiff_t>);

    static_assert(!std::is_invocable_v<L, bool>);
    static_assert(!std::is_invocable_v<L, char>);
    static_assert(!std::is_invocable_v<L, wchar_t>);
#ifndef TEST_HAS_NO_CHAR8_T
    static_assert(!std::is_invocable_v<L, char8_t>);
#endif
    static_assert(!std::is_invocable_v<L, char16_t>);
    static_assert(!std::is_invocable_v<L, char32_t>);

    static_assert(!std::is_invocable_v<L, A>);
    static_assert(!std::is_invocable_v<L, A*>);
    static_assert(!std::is_invocable_v<L, E1>);
    static_assert(!std::is_invocable_v<L, E2>);
    }

    static_assert(test<unsigned char>());
    static_assert(test<unsigned short>());
    static_assert(test<unsigned int>());
    static_assert(test<unsigned long>());
    static_assert(test<unsigned long long>());
#ifndef TEST_HAS_NO_INT128
    static_assert(test<__uint128_t>());
#endif
    static_assert(test<uint8_t>());
    static_assert(test<uint16_t>());
    static_assert(test<uint32_t>());
    static_assert(test<uint64_t>());
    static_assert(test<uintmax_t>());
    static_assert(test<uintptr_t>());
    static_assert(test<size_t>());

    test<unsigned char>();
    test<unsigned short>();
    test<unsigned int>();
    test<unsigned long>();
    test<unsigned long long>();
#ifndef TEST_HAS_NO_INT128
    test<__uint128_t>();
#endif
    test<uint8_t>();
    test<uint16_t>();
    test<uint32_t>();
    test<uint64_t>();
    test<uintmax_t>();
    test<uintptr_t>();
    test<size_t>();

    return 0;
}
