#include "str.h"
#include "Vector.h"

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

size_t strlen(String str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}
// Compares two strings. Returns 0 if they are exactly the same.
int strcmp(const char* s1, const char* s2) {
    while (*s1 != '\0' && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

// Compares up to 'n' characters of two strings.
int strncmp(const char* s1, const char* s2, size_t n) {
    if (n == 0) return 0;
    
    while (n-- > 0 && *s1 != '\0' && (*s1 == *s2)) {
        if (n == 0) return 0;
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

// Copies the string from src to dest, including the '\0'
char* strcpy(char* dest, const char* src) {
    char* original_dest = dest;
    while ((*dest++ = *src++) != '\0') {
        // The loop does the copying automatically
    }
    return original_dest;
}


String::String() {
    slength = 0;
    buffer.append('\0'); // Always keep it null-terminated
}

String::String(const char* str) {
    slength = 0;
    while (*str) {
        buffer.append(*str++);
        slength++;
    }
    buffer.append('\0');
}



void String::add(char c) {
    // Remove old null terminator, add char, add new null terminator
    slength++;
    buffer.deleteAt(buffer.size() - 1);
    buffer.append(c);
    buffer.append('\0');
}

size_t String::size() const {
    return buffer.size() - 1; // Don't count the \0
}

char* String::c_str() {
    return &buffer[0]; // Returns pointer to the first char
}

String& String::operator+=(char c) {
    add(c);
    return *this;
}

bool String::operator==(const char* other) {
    return strcmp(this->c_str(), other) == 0;
}

bool String::operator==(const String& other) {
    return strcmp(this->c_str(), ((String&)other).c_str()) == 0;
}

String String::operator+(const String& other) {
    String res = *this;
    for (size_t i = 0; i < other.size(); i++) {
        // Accessing buffer[i] from the 'other' string
        res.add(((String&)other).buffer[i]); 
    }
    return res;
}

String String::operator+(const char* other) {
    String res = *this;
    while (*other) {
        res.add(*other++);
    }
    return res;
}

String operator+(const char* lhs, const String& rhs) {
    String res = lhs;
    return res + rhs;
}


String& String::operator=(const char* str) {
    // Clear current buffer
    while(buffer.size() > 0) buffer.deleteAt(0);
    
    while (*str) {
        buffer.append(*str++);
        slength++;
    }
    buffer.append('\0');
    return *this;
}

String& String::operator=(const String& other) {
    if (this != &other) {
        buffer = other.buffer;
        slength = other.slength;
    }
    return *this;
}

char& String::operator[](size_t index) {
    return buffer[index];
}

const char& String::operator[](size_t index) const {
    return ((const Vector<char>&)buffer)[index];
}

uint_32 String::length(){
    return slength;
}

void String::ToUpper() {
    for (size_t i = 0; i < slength; i++) {
        if (buffer[i] >= 'a' && buffer[i] <= 'z') {
            buffer[i] = buffer[i] - 'a' + 'A';
        }
    }
}

void String::Trim() {
    // 1. Right trim
    while (slength > 0 && (buffer[slength - 1] == ' ' || buffer[slength - 1] == '\t' || buffer[slength - 1] == '\r' || buffer[slength - 1] == '\n')) {
        buffer.deleteAt(slength); // Remove null terminator
        buffer.deleteAt(slength - 1); // Remove the space
        buffer.append('\0'); // Restore null terminator
        slength--;
    }

    // 2. Left trim
    while (slength > 0 && (buffer[0] == ' ' || buffer[0] == '\t' || buffer[0] == '\r' || buffer[0] == '\n')) {
        buffer.deleteAt(0);
        slength--;
    }
}

// Assuming you have: 
// Vector<String> 
// String::String(const char*) constructor
// String::append(char) or String += char

Vector<String> split(String text, char delimiter) {
    Vector<String> segments;
    String currentSegment = "";

    for (int i = 0; i < text.length(); i++) {
        char c = text[i];

        if (c == delimiter) {
            // Only add if the segment isn't empty (avoids empty strings from double spaces)
            if (currentSegment.length() > 0) {
                segments.append(currentSegment);
                currentSegment = ""; // Reset for next word
            }
        } else {
            currentSegment.add(c);
        }
    }

    // Don't forget to add the very last segment after the loop
    if (currentSegment.length() > 0) {
        segments.append(currentSegment);
    }

    return segments;
}
