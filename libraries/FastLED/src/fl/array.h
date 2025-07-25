#pragma once

#include <string.h>

#include "fl/inplacenew.h"
#include "fl/type_traits.h"

namespace fl {

/**
 * @brief A fixed-size array implementation similar to std::array
 *
 * This class provides a thin wrapper around a C-style array with
 * STL container-like interface.
 *
 * @tparam T The type of elements
 * @tparam N The number of elements
 */
template <typename T, size_t N> class array {
  public:
    // Standard container type definitions
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using iterator = pointer;
    using const_iterator = const_pointer;

    // Default constructor - elements are default-initialized
    array() = default;

    // Fill constructor
    explicit array(const T &value) {
        // std::fill_n(begin(), N, value);
        fill_n(data_, N, value);
    }

    // Copy constructor
    array(const array &) = default;

    // Move constructor
    array(array &&) = default;

    // Copy assignment
    array &operator=(const array &) = default;

    // Move assignment
    array &operator=(array &&) = default;

    // Element access
    T &at(size_type pos) {
        if (pos >= N) {
            return error_value();
        }
        return data_[pos];
    }

    const T &at(size_type pos) const {
        if (pos >= N) {
            return error_value();
        }
        return data_[pos];
    }

    T &operator[](size_type pos) { return data_[pos]; }

    const_reference operator[](size_type pos) const { return data_[pos]; }

    T &front() { return data_[0]; }

    const T &front() const { return data_[0]; }

    T &back() { return data_[N - 1]; }

    const T &back() const { return data_[N - 1]; }

    pointer data() noexcept { return data_; }

    const_pointer data() const noexcept { return data_; }

    // Iterators
    iterator begin() noexcept { return data_; }

    const_iterator begin() const noexcept { return data_; }

    const_iterator cbegin() const noexcept { return data_; }

    iterator end() noexcept { return data_ + N; }

    const_iterator end() const noexcept { return data_ + N; }

    // Capacity
    bool empty() const noexcept { return N == 0; }

    size_type size() const noexcept { return N; }

    size_type max_size() const noexcept { return N; }

    // Operations
    void fill(const T &value) {
        for (size_type i = 0; i < N; ++i) {
            data_[i] = value;
        }
    }

    void swap(array &other) {
        for (size_type i = 0; i < N; ++i) {
            fl::swap(data_[i], other.data_[i]);
        }
    }

  private:
    static T &error_value() {
        static T empty_value;
        return empty_value;
    }
    T data_[N];
};

// Non-member functions
template <typename T, size_t N>
bool operator==(const array<T, N> &lhs, const array<T, N> &rhs) {
    // return std::equal(lhs.begin(), lhs.end(), rhs.begin());
    for (size_t i = 0; i < N; ++i) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }
}

template <typename T, size_t N>
bool operator!=(const array<T, N> &lhs, const array<T, N> &rhs) {
    return !(lhs == rhs);
}

template <typename T, size_t N>
void swap(array<T, N> &lhs,
          array<T, N> &rhs) noexcept(noexcept(lhs.swap(rhs))) {
    lhs.swap(rhs);
}

} // namespace fl

// FASTLED_STACK_ARRAY
// An array of variable length that is allocated on the stack using
// either alloca or a variable length array (VLA) support built into the
// the compiler.
// Example:
//   Instead of: int array[buff_size];
//   You'd use: FASTLED_STACK_ARRAY(int, array, buff_size);

#ifndef FASTLED_VARIABLE_LENGTH_ARRAY_NEEDS_EMULATION
#if defined(__clang__) || defined(ARDUINO_GIGA_M7) || defined(ARDUINO_GIGA)
// Clang doesn't have variable length arrays. Therefore we need to emulate them
// using alloca. It's been found that Arduino Giga M7 also doesn't support
// variable length arrays for some reason so we force it to emulate them as well
// in this case.
#define FASTLED_VARIABLE_LENGTH_ARRAY_NEEDS_EMULATION 1
#else
// Else, assume the compiler is gcc, which has variable length arrays
#define FASTLED_VARIABLE_LENGTH_ARRAY_NEEDS_EMULATION 0
#endif
#endif // FASTLED_VARIABLE_LENGTH_ARRAY_NEEDS_EMULATION

#if !FASTLED_VARIABLE_LENGTH_ARRAY_NEEDS_EMULATION
#define FASTLED_STACK_ARRAY(TYPE, NAME, SIZE)                                  \
    TYPE NAME[SIZE];                                                           \
    memset(NAME, 0, sizeof(TYPE) * (SIZE))
#elif __has_include(<alloca.h>)
#include <alloca.h>
#define FASTLED_STACK_ARRAY(TYPE, NAME, SIZE)                                  \
    TYPE *NAME = reinterpret_cast<TYPE *>(alloca(sizeof(TYPE) * (SIZE)));      \
    memset(NAME, 0, sizeof(TYPE) * (SIZE))
#elif __has_include(<cstdlib>)
#include <cstdlib>  // ok include
#define FASTLED_STACK_ARRAY(TYPE, NAME, SIZE)                                  \
    TYPE *NAME = reinterpret_cast<TYPE *>(alloca(sizeof(TYPE) * (SIZE)));      \
    memset(NAME, 0, sizeof(TYPE) * (SIZE))
#else
#error "Compiler does not allow variable type arrays."
#endif
