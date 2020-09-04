#ifndef JSPX_UTILS_H
#define JSPX_UTILS_H

#include "jspx.h"
#include <string>

JSPX_NAMESPACE_BEGIN

class ChWrapper {

private:
    const char* src_;
    const char* current_;
    bool is_dynamic_;
    uint8_t offset_;

private:
    uint8_t GetLength(const char* ch) const {
        uint8_t length = 0;
        while (*ch++ != '\0')
            length++;
        return length;
    }

    const char* DeepClone(const char* source) const {
        uint8_t length = GetLength(source);
        char* target = new char[length + 1];
        for (uint8_t i = 0; i < length; i++)
            target[i] = source[i];
        target[length] = '\0';
        return target;
    }

public:
    ChWrapper(const ChWrapper&) = delete;
    ChWrapper& operator= (const ChWrapper& rhs) {
        this->~ChWrapper();
        if (!rhs.src_)
            return *this;

        const char* tmp = DeepClone(rhs.src_);
        current_ = tmp;
        src_ = tmp;
        offset_ = 0;
        is_dynamic_ = true;
        return *this;
    };

    ChWrapper(ChWrapper&& rhs) = delete;
    ChWrapper& operator = (ChWrapper&& rhs) noexcept {
        this->~ChWrapper();
        if (!rhs.src_)
            return *this;

        const char* tmp = DeepClone(rhs.src_);
        current_ = tmp;
        src_ = tmp;
        offset_ = 0;
        is_dynamic_ = true;
        return *this;
    };

    ChWrapper() :
        src_(0),
        current_(0),
        offset_(0),
        is_dynamic_(false)
    { }

    ChWrapper(const char* ch) :
        src_(ch),
        current_(ch),
        offset_(0),
        is_dynamic_(false)
    { }

    ChWrapper(const std::string& s) {
        uint8_t length = s.length();
        char* temp = new char[length + 1];
        for (uint8_t i = 0; i < length; i++)
            temp[i] = s[i];
        temp[length] = '\0';

        current_ = temp;
        src_ = temp;
        offset_ = 0;
        is_dynamic_ = true;
    }

    ~ChWrapper() {
        if (is_dynamic_)
            delete[] current_;
    }

    const char Peek() const {
        return *current_;
    }

    const char Pop() {
        offset_++;
        return *current_++;
    }

    uint8_t GetOffset() const {
        return offset_;
    }

    std::string GetString() const {
        if (!src_)
            return std::string();
        return std::string(src_);
    }

    void Flush() {
        src_ = NULL;
        current_ = NULL;
        offset_ = 0;
        is_dynamic_ = false;
    }

    void SkipWhitespace(ChWrapper& ch) const {
        while (ch.Peek() == ' ' || ch.Peek() == '\n' || ch.Peek() == '\t')
            ch.Pop();
    }

    std::string GetCurrentSnippet(uint8_t offset) const {
        if (!current_)
            return std::string();

        const char* low = current_;
        const char* high = current_;

        for (uint8_t i = 0; i < std::min(offset, GetOffset()); i++) {
            if (*low == '\0') {
                break;
            }
            low--;
        }
        for (uint8_t i = 0; i < offset; i++) {
            if (*high == '\0') {
                break;
            }
            high++;
        }
        uint8_t length = high - low;
        return std::string(low, length);
    }
};

JSPX_NAMESPACE_END

#endif // !JSPX_UTILS_H
