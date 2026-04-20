#pragma once

#include "TypeDefs.h"
#include "Heap.h" 

template <typename T>
class Vector {
private:
    T* contents;
    size_t capacity;
    size_t current_size;

    void grow() {
        capacity = (capacity == 0) ? 4 : capacity * 2;
        T* new_contents = new T[capacity];
        for (size_t i = 0; i < current_size; i++) {
            new_contents[i] = contents[i];
        }
        if (contents) delete[] contents;
        contents = new_contents;
    }

public:
    Vector() : contents(nullptr), capacity(0), current_size(0) {}

    // Copy Constructor (Deep Copy)
    Vector(const Vector& other) {
        capacity = other.capacity;
        current_size = other.current_size;
        if (capacity > 0) {
            contents = new T[capacity];
            for (size_t i = 0; i < current_size; i++) {
                contents[i] = other.contents[i];
            }
        } else {
            contents = nullptr;
        }
    }

    // Assignment Operator (Deep Copy)
    Vector& operator=(const Vector& other) {
        if (this != &other) {
            if (contents) delete[] contents;
            capacity = other.capacity;
            current_size = other.current_size;
            if (capacity > 0) {
                contents = new T[capacity];
                for (size_t i = 0; i < current_size; i++) {
                    contents[i] = other.contents[i];
                }
            } else {
                contents = nullptr;
            }
        }
        return *this;
    }

    void append(T item) {
        if (current_size == capacity) grow();
        contents[current_size++] = item;
    }

    void deleteAt(size_t index) {
        if (index >= current_size) return;
        for (size_t i = index; i < current_size - 1; i++) {
            contents[i] = contents[i + 1];
        }
        current_size--;
    }

    T& operator[](size_t index) { 
        return contents[index]; 
    }

    const T& operator[](size_t index) const {
        return contents[index];
    }
    
    size_t size() const { 
        return current_size; 
    }

    ~Vector() {
        if (contents) delete[] contents;
    }
};
