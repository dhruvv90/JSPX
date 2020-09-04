#ifndef JSPX_ERROR_H
#define JSPX_ERROR_H

#include "jspx.h"
#include "utils.h"
#include <exception>
#include <unordered_map>

JSPX_NAMESPACE_BEGIN

class Throwable {

protected:
    static void ThrowInvalidIdentifier(const ChWrapper& ch, uint8_t offset = 10) {
        std::string details = ch.GetCurrentSnippet(offset);
        std::string message = "Invalid Identifier in input at : ";
        throw std::exception(message.append(details).c_str());
    }

    static void ThrowInvalidMethodCall() {
        throw std::exception("Invalid Method Call");
    }

    static void ThrowUnsupportedOperation() {
        throw std::exception("Unsupported Operation");
    }
};

JSPX_NAMESPACE_END

#endif // !JSPX_ERROR_H
