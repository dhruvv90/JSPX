#ifndef JSPX_UTILS_H
#define JSPX_UTILS_H

#include "jspx.h"
#include <string>

JSPX_NAMESPACE_BEGIN

class ChWrapper {

private:
    const char* src_;
    const char* current_;
    size_t offset_;
    bool is_dynamic_;

public:
    ChWrapper(const ChWrapper&) = delete;
    ChWrapper& operator = (const ChWrapper& rhs) = delete;
    ChWrapper(ChWrapper&& rhs) = delete;

    ChWrapper& operator = (ChWrapper&& rhs) noexcept {
        this->~ChWrapper();

        size_t size = 0;
        {
            const char* temp = rhs.current_;
            while (*temp++ != '\0') {
                size++;
            }
        }

        auto temp = new char[size];
        for (size_t i = 0; i < size; i++) {
            temp[i] = rhs.current_[i];
        }
        current_ = temp;
        src_ = temp;
        offset_ = 0;
        is_dynamic_ = true;

        return *this;
    };

    ChWrapper() :
        src_(0),
        current_(0),
        offset_(0),
        is_dynamic_(false) { }

    ChWrapper(const char* ch) :
        src_(ch),
        current_(ch),
        offset_(0),
        is_dynamic_(false) { }

    ChWrapper(const char* ch, size_t s) :
        src_(ch),
        offset_(0),
        is_dynamic_(true) {
        auto temp = new char[s];
        for (size_t i = 0; i < s; i++) {
            temp[i] = ch[i];
        }
        current_ = temp;
    }

    ChWrapper(const char* ch, bool createCopy) :
        src_(ch),
        offset_(0),
        is_dynamic_(createCopy) {
        if (!createCopy) {
            ChWrapper::ChWrapper(ch);
        }
        else {
            size_t size;
            const char* temp = ch;
            while (*temp++ != '\0') {
                size++;
            }
            ChWrapper::ChWrapper(ch, size);
        }
    }

    ~ChWrapper() {
        if (is_dynamic_) {
            delete[] current_;
        }
    }

    const char Peek() const {
        return *current_;
    }

    const char Pop() {
        offset_++;
        return *current_++;
    }

    size_t GetOffset() const {
        return offset_;
    }

    void SkipWhitespace(ChWrapper& ch) const {
        while (ch.Peek() == ' ' || ch.Peek() == '\n' || ch.Peek() == '\t')
            ch.Pop();
    }

    std::string GetSnippet(size_t offset) const {
        const char* low = current_;
        const char* high = current_;

        for (size_t i = 0; i < std::min(offset, GetOffset()); i++) {
            if (*low == '\0') {
                break;
            }
            low--;
        }
        for (size_t i = 0; i < offset; i++) {
            if (*high == '\0') {
                break;
            }
            high++;
        }
        size_t length = high - low;
        return std::string(low, length);
    }
};

JSPX_NAMESPACE_END

#endif // !JSPX_UTILS_H
