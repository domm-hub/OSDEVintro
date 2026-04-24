#include "str.h"
#include "Vector.h"

size_t strlen(const char* str) {
    size_t len = 0;
    while (str && str[len] != '\0') {
        len++;
    }
    return len;
}

size_t strlen(String str) {
    return str.length();
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 != '\0' && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    if (n == 0) return 0;
    while (n-- > 0 && *s1 != '\0' && (*s1 == *s2)) {
        if (n == 0) return 0;
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

char* strcpy(char* dest, const char* src) {
    char* original_dest = dest;
    while ((*dest++ = *src++) != '\0');
    return original_dest;
}

uint_64 StringToInt(const char* str) {
    uint_64 res = 0;
    while (*str) {
        if (*str >= '0' && *str <= '9') {
            res = res * 10 + (*str - '0');
        }
        str++;
    }
    return res;
}

uint_64 StringToInt(String str) {
    return StringToInt(str.c_str());
}

String::String() {
    slength = 0;
    buffer.append('\0');
}

String::String(const char* str) {
    slength = 0;
    if (str) {
        while (*str) {
            buffer.append(*str++);
            slength++;
        }
    }
    buffer.append('\0');
}

void String::add(char c) {
    buffer.deleteAt(buffer.size() - 1);
    buffer.append(c);
    buffer.append('\0');
    slength++;
}

size_t String::size() const {
    return slength;
}

char* String::c_str() {
    return &buffer[0];
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

bool String::operator!=(const char* other) {
    return strcmp(this->c_str(), other) != 0;
}

bool String::operator!=(const String& other) {
    return strcmp(this->c_str(), ((String&)other).c_str()) != 0;
}

String String::operator+(const String& other) {
    String res = *this;
    for (size_t i = 0; i < other.size(); i++) {
        res.add(((String&)other).buffer[i]); 
    }
    return res;
}

String String::operator+(const char* other) {
    String res = *this;
    if (other) {
        while (*other) {
            res.add(*other++);
        }
    }
    return res;
}

String operator+(const char* lhs, const String& rhs) {
    String res = lhs;
    return res + rhs;
}

String& String::operator=(const char* str) {
    buffer.clear();
    slength = 0;
    if (str) {
        while (*str) {
            buffer.append(*str++);
            slength++;
        }
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
    if (slength == 0) return;

    // Right trim
    while (slength > 0) {
        char c = buffer[slength - 1];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            buffer.deleteAt(slength); // Remove \0
            buffer.deleteAt(slength - 1); // Remove space
            buffer.append('\0');
            slength--;
        } else {
            break;
        }
    }

    // Left trim
    while (slength > 0) {
        char c = buffer[0];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            buffer.deleteAt(0);
            slength--;
        } else {
            break;
        }
    }
}

Vector<String> split(String text, char delimiter) {
    Vector<String> segments;
    String currentSegment = "";

    for (uint_32 i = 0; i < text.length(); i++) {
        char c = text[i];
        if (c == delimiter) {
            if (currentSegment.length() > 0) {
                segments.append(currentSegment);
                currentSegment = ""; 
            }
        } else {
            currentSegment.add(c);
        }
    }

    if (currentSegment.length() > 0) {
        segments.append(currentSegment);
    }

    return segments;
}
