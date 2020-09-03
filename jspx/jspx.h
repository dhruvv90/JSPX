#ifndef JSPX_H
#define JSPX_H

#define JSPX_NAMESPACE jspx

#define JSPX_NAMESPACE_BEGIN namespace JSPX_NAMESPACE {
#define JSPX_NAMESPACE_END }


JSPX_NAMESPACE_BEGIN
/*
Common Preprocessor / macro declarations for the project can be declared here.
*/

enum Identifier {
    kOpenSquareBracket = '[',
    kCloseSquareBracket = ']',
    kOpenCurlyBracket = '{',
    kCloseCurlyBracket = '}',

    kColon = ':',
    kComma = ',',
    kQuote = '"',
};


enum ParsingType {
    kParsingTypeIterative,
    kParsingTypeRecursive
};


JSPX_NAMESPACE_END

#endif // !JSPX_H_
