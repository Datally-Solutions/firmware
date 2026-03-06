#pragma once
#include <cmath>
#include <cstring>
#include <sstream>
#include <string>

// Mock Arduino String with needed methods
class String : public std::string {
   public:
    String() : std::string() {}
    String(const char* s) : std::string(s ? s : "") {}
    String(const std::string& s) : std::string(s) {}
    String(int v) : std::string(std::to_string(v)) {}
    String(unsigned long v) : std::string(std::to_string(v)) {}
    String(long v) : std::string(std::to_string(v)) {}
    String(float v, int dec = 2) {
        std::ostringstream ss;
        ss << std::fixed;
        ss.precision(dec);
        ss << v;
        this->assign(ss.str());
    }
    String(double v, int dec = 2) {
        std::ostringstream ss;
        ss << std::fixed;
        ss.precision(dec);
        ss << v;
        this->assign(ss.str());
    }

    // Arduino String methods
    int indexOf(const char* s) const {
        size_t pos = this->find(s);
        return pos == std::string::npos ? -1 : (int)pos;
    }

    String substring(int from, int to = -1) const {
        if (to == -1) return String(this->substr(from));
        return String(this->substr(from, to - from));
    }

    void replace(const char* from, const char* to) {
        size_t pos = 0;
        while ((pos = this->find(from, pos)) != std::string::npos) {
            std::string::replace(pos, strlen(from), to);
            pos += strlen(to);
        }
    }

    int length() const { return (int)this->size(); }

    // Operators
    bool operator==(const char* s) const { return this->compare(s) == 0; }
    bool operator!=(const char* s) const { return this->compare(s) != 0; }
    bool operator==(const String& s) const { return this->compare(s) == 0; }
    bool operator!=(const String& s) const { return this->compare(s) != 0; }

    String operator+(const String& s) const { return String(std::string(*this) + std::string(s)); }
    String operator+(const char* s) const { return String(std::string(*this) + s); }
    String operator+(int v) const { return String(std::string(*this) + std::to_string(v)); }
    String operator+(unsigned long v) const {
        return String(std::string(*this) + std::to_string(v));
    }
    String operator+(float v) const { return *this + String(v); }

    String& operator+=(const String& s) {
        std::string::operator+=(s);
        return *this;
    }
    String& operator+=(const char* s) {
        std::string::operator+=(s);
        return *this;
    }
};

// Allow "literal" + String
inline String operator+(const char* lhs, const String& rhs) {
    return String(std::string(lhs) + std::string(rhs));
}

// Mock Serial
struct SerialClass {
    void println(const String& s) {}
    void println(const char* s) {}
    void begin(int baud) {}
};
inline SerialClass Serial;

// Mock types
typedef unsigned int uint32_t;

// Mock millis
inline unsigned long millis() { return 0; }
