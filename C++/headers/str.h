#pragma once
#include "TypeDefs.h"
#include "Vector.h"

size_t strlen(const char* str);


int strcmp(const char* s1, const char* s2);

int strncmp(const char* s1, const char* s2, size_t n);

char* strcpy(char* dest, const char* src);

uint_64 StringToInt(const char* str);

class String {

private:
    Vector<char> buffer;
    uint_32 slength;

public:
    // Constructors
    String();
    String(const char* str);

    // Methods
    void add(char c);
    size_t size() const;
    char* c_str(); // Returns raw pointer

    // Operators
    String& operator+=(char c);
    bool operator==(const char* other);
    bool operator==(const String& other);
    bool operator!=(const char* other);
    bool operator!=(const String& other);
    String operator+(const String& other);
    String operator+(const char* other);
    String& operator=(const char* str); 
    String& operator=(const String& other);

    char& operator[](size_t index);
    const char& operator[](size_t index) const;

    void ToUpper();
    void Trim();

    uint_32 length();
    
    // Allows kprint(myString)
    operator char*() { return c_str(); }
    };

    uint_64 StringToInt(String str);

    String operator+(const char* lhs, const String& rhs);