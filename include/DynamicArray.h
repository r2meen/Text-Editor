#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <cstddef>
#include <stdexcept>
#include <utility>

// ------------------------------------------------------------
// DynamicArray<T>  -  hand-written resizable array
// Capacity doubles when full, so push_back is O(1) amortised.
// Used for search results and match lists (replace / delete).
// ------------------------------------------------------------

template <typename T>
class DynamicArray {
private:
    T* data;
    std::size_t count;
    std::size_t cap;

    void grow() {
        std::size_t newCap = (cap == 0) ? 4 : cap * 2;
        T* bigger = new T[newCap];

        for (std::size_t i = 0; i < count; i++)
            bigger[i] = std::move(data[i]);

        delete[] data;
        data = bigger;
        cap = newCap;
    }

public:
    DynamicArray() : data(nullptr), count(0), cap(0) {}

    ~DynamicArray() { delete[] data; }

    // Deep copy
    DynamicArray(const DynamicArray& other)
        : data(nullptr), count(0), cap(0) {
        for (std::size_t i = 0; i < other.count; i++)
            push_back(other.data[i]);
    }

    DynamicArray& operator=(const DynamicArray& other) {
        if (this != &other) {
            DynamicArray copy(other);
            std::swap(data, copy.data);
            std::swap(count, copy.count);
            std::swap(cap, copy.cap);
        }
        return *this;
    }

    // Move (steals the buffer)
    DynamicArray(DynamicArray&& other) noexcept
        : data(other.data), count(other.count), cap(other.cap) {
        other.data = nullptr;
        other.count = other.cap = 0;
    }

    DynamicArray& operator=(DynamicArray&& other) noexcept {
        if (this != &other) {
            delete[] data;
            data = other.data;
            count = other.count;
            cap = other.cap;
            other.data = nullptr;
            other.count = other.cap = 0;
        }
        return *this;
    }

    void push_back(const T& value) {
        if (count == cap) grow();
        data[count++] = value;
    }

    T& operator[](std::size_t i) {
        if (i >= count) throw std::out_of_range("DynamicArray index");
        return data[i];
    }

    const T& operator[](std::size_t i) const {
        if (i >= count) throw std::out_of_range("DynamicArray index");
        return data[i];
    }

    std::size_t size() const { return count; }
    bool empty() const { return count == 0; }
    void clear() { count = 0; }

    // Allows range-based for loops
    T* begin() { return data; }
    T* end() { return data + count; }
    const T* begin() const { return data; }
    const T* end() const { return data + count; }
};

#endif
