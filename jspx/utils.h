#ifndef JSPX_UTILS_H
#define JSPX_UTILS_H

#include "jspx.h"
#include <string>


JSPX_NAMESPACE_BEGIN

class ChWrapper {

public:
	ChWrapper(const char* ch) : src_(ch), current_(ch), offset_(0) {

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


private:
	const char* const src_;
	const char* current_;
	size_t offset_;
};

JSPX_NAMESPACE_END

#endif // !JSPX_UTILS_H
