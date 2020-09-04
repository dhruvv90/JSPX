#ifndef JSPX_DOCUMENT_H
#define JSPX_DOCUMENT_H

#include "jspx.h"
#include "error.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <cctype>

JSPX_NAMESPACE_BEGIN

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


class Entity : public virtual Throwable {

private:
    typedef typename std::unique_ptr<Entity> EntitySP;
    enum Type {
        kTypeObject,
        kTypeArray,
        kTypeString,
        kTypeNumber,
        kTypeBoolean,
        kTypeNull,
        kTypeEmpty
    };

    struct Vault {
        std::string s;
        double d;
        bool b;

        void Clear() {
            s.clear();
            d = NULL;
            b = false;
        }
    } vault_;

    Type type_;
    std::unordered_map<std::string, EntitySP> o_members_;
    std::vector<EntitySP> a_members_;

public:
    Entity(const Entity& rhs) = delete;
    Entity& operator = (const Entity& rhs) = delete;

    Entity& operator = (Entity&& rhs) noexcept {
        if (*this == rhs) {
            return *this;
        }
        Flush();
        o_members_ = std::move(rhs.o_members_);
        a_members_ = std::move(rhs.a_members_);
        type_ = rhs.type_;
        vault_ = rhs.vault_;
        rhs.Flush();
        return *this;
    };

    Entity(Entity&& rhs) noexcept {
        if (*this == rhs) {
            return;
        }
        Flush();
        o_members_ = std::move(rhs.o_members_);
        a_members_ = std::move(rhs.a_members_);
        type_ = rhs.type_;
        vault_ = rhs.vault_;
        rhs.Flush();
    };

    bool operator ==(const Entity& rhs) const {
        if (type_ != rhs.type_) {
            return false;
        }
        switch (type_)
        {
        case kTypeObject:
            return o_members_ == rhs.o_members_;
        case kTypeArray:
            return a_members_ == rhs.a_members_;
        default:
            return (vault_.s == rhs.vault_.s) && (vault_.d == rhs.vault_.d) && (vault_.b == rhs.vault_.b);
        }
    }

public:
    Entity() : type_(kTypeEmpty) {	}

    explicit Entity(std::string s) : type_(kTypeString) {
        vault_.s = s;
    }

    explicit Entity(const char* ch) : type_(kTypeString) {
        vault_.s = ch;
    }

    explicit Entity(const double d) : type_(kTypeNumber) {
        vault_.d = d;
    }

    explicit Entity(bool b) : type_(kTypeBoolean) {
        vault_.b = b;
    }

private:
    explicit Entity(Type t) : type_(t) {
        switch (t)
        {
        case kTypeObject:
            break;
        case kTypeArray:
            break;
        case kTypeString:
            break;
        case kTypeNull:
            break;
        default:
            ThrowInvalidMethodCall();
        }
    }

protected:
    void Flush() {
        SetType(kTypeEmpty);
        o_members_.clear();
        a_members_.clear();
        vault_.Clear();
    }

    bool IsEndOfStream(ChWrapper& ch) const {
        try {
            EnsureChar(ch, '\0');
        }
        catch (...) {
            return false;
        }
        return true;
    }

public:
    bool IsObject() const {
        return type_ == kTypeObject;
    }

    bool IsArray() const {
        return type_ == kTypeArray;
    }

    bool IsString() const {
        return type_ == kTypeString;
    }

    bool IsNumber() const {
        return type_ == kTypeNumber;
    }

    bool IsBoolean() const {
        return type_ == kTypeBoolean;
    }

    bool IsNull() const {
        return type_ == kTypeNull;
    }

    std::string GetString() const {
        if (IsString())
            return vault_.s;
        ThrowInvalidMethodCall();
    }

    const bool GetBool() const {
        if (IsBoolean())
            return vault_.b;
        ThrowInvalidMethodCall();
    }

    const double GetNumber() const {
        if (IsNumber())
            return vault_.d;
        ThrowInvalidMethodCall();
    }

    void SetNumber(double d) {
        Flush();
        this->SetType(kTypeNumber);
        this->vault_.d = d;
    }

    void SetString(std::string s) {
        Flush();
        this->SetType(kTypeString);
        this->vault_.s = std::move(s);
    }

    void SetBool(bool b) {
        Flush();
        this->SetType(kTypeBoolean);
        this->vault_.b = b;
    }

    void SetNull() {
        Flush();
        this->SetType(kTypeNull);
        this->vault_.Clear();
    }

    bool HasMember(const std::string& key) const {
        return o_members_.count(key) > 0;
    }

    /*void AddObjectMember(const std::string& key, const Entity& value) {
        if (!IsObject()) {
            ThrowInvalidMethodCall();
        }
        o_members_.insert_or_assign(key, std::make_unique<Entity>(std::move(value)));
    }

    void AddArrayMember(const Entity& value) {
        if (!IsArray()) {
            ThrowInvalidMethodCall();
        }
        a_members_.push_back(std::make_unique<Entity>(std::move(value)));
    }*/

    Entity& operator [](const std::string& key) const {
        if (!IsObject()) {
            ThrowInvalidMethodCall();
        }
        return *o_members_.at(key).get();
    }

    Entity& operator [](size_t idx) const {
        if (!IsArray()) {
            ThrowInvalidMethodCall();
        }
        return *a_members_.at(idx).get();
    }

private:
    void SetType(Type t) {
        type_ = t;
    }

    void SkipWhitespace(ChWrapper& ch) const {
        while (ch.Peek() == ' ' || ch.Peek() == '\n' || ch.Peek() == '\t')
            ch.Pop();
    }

    void EnsureChar(ChWrapper& ch, Identifier target, bool consume = false) const {
        if (ch.Peek() != static_cast<const char>(target)) {
            ThrowInvalidIdentifier(ch);
        }
        if (consume)
            ch.Pop();
    }

    void EnsureChar(ChWrapper& ch, const char& target, bool consume = false) const {
        if (ch.Peek() != target) {
            ThrowInvalidIdentifier(ch);
        }
        if (consume)
            ch.Pop();
    }

    bool CompareIdentifier(ChWrapper& ch, Identifier idf, bool consumeIfMatch = false) const {
        bool result = ch.Peek() == static_cast<const char>(idf);
        if (result && consumeIfMatch)
            ch.Pop();
        return result;
    }

    bool CheckChar(ChWrapper& ch, const char& target, bool consumeIfMatch = false) const
    {
        bool result = ch.Peek() == target;
        if (consumeIfMatch && result) {
            ch.Pop();
        }
        return result;
    }

    void ParseTrue(ChWrapper& ch, Entity& e) const {
        ParseStringLiteral(ch, "true");
        e.SetType(kTypeBoolean);
        e.vault_.b = true;
    }

    void ParseFalse(ChWrapper& ch, Entity& e) const {
        ParseStringLiteral(ch, "false");

        e.SetType(kTypeBoolean);
        e.vault_.b = false;
    }

    void ParseNull(ChWrapper& ch, Entity& e) const {
        ParseStringLiteral(ch, "null");

        e.SetType(kTypeNull);
    }

    void ParseString(ChWrapper& ch, Entity& e) const {
        EnsureChar(ch, kQuote, true);
        std::string word;

        while (!CompareIdentifier(ch, kQuote, true)) {
            if (ch.Peek() == '\0')
                ThrowInvalidIdentifier(ch);
            word.push_back(ch.Pop());
        }

        e.SetType(kTypeString);
        e.vault_.s = std::move(word);
    }

    void ParseObject(ChWrapper& ch, Entity& e) const {
        e.SetType(kTypeObject);
        EnsureChar(ch, kOpenCurlyBracket, true);
        SkipWhitespace(ch);

        if (CompareIdentifier(ch, kCloseCurlyBracket, true))
            return;
        SkipWhitespace(ch);

        while (true) {

            Entity key;
            ParseString(ch, key);
            SkipWhitespace(ch);

            EnsureChar(ch, kColon, true);
            SkipWhitespace(ch);

            Entity value;
            ParseValue(ch, value);
            SkipWhitespace(ch);

            e.o_members_.insert_or_assign(
                key.GetString(),
                std::make_unique<Entity>(std::move(value))
            );

            if (CompareIdentifier(ch, kCloseCurlyBracket, true))
                break;

            EnsureChar(ch, kComma, true);
            SkipWhitespace(ch);
        }
    }

    void ParseArray(ChWrapper& ch, Entity& e) const {
        e.SetType(kTypeArray);
        EnsureChar(ch, kOpenSquareBracket, true);
        SkipWhitespace(ch);
        if (CompareIdentifier(ch, kCloseSquareBracket, true))
            return;
        SkipWhitespace(ch);

        while (true) {

            Entity member;
            ParseValue(ch, member);
            SkipWhitespace(ch);
            e.a_members_.push_back(std::make_unique<Entity>(std::move(member)));

            SkipWhitespace(ch);
            if (CompareIdentifier(ch, kCloseSquareBracket, true))
                break;

            EnsureChar(ch, kComma, true);
            SkipWhitespace(ch);
        }
    }

    void ParseNumber(ChWrapper& ch, Entity& e) const {
        SkipWhitespace(ch);
        try {
            if (CheckChar(ch, '-') || (std::isdigit(ch.Peek()) && !CheckChar(ch, '0')))
            {
                e.SetType(kTypeNumber);

                std::string number;
                bool hasDecimal = false;
                number.push_back(ch.Pop());
                SkipWhitespace(ch);

                while (std::isdigit(ch.Peek()) || CheckChar(ch, '.')) {
                    if (CheckChar(ch, '.')) {
                        if (hasDecimal) {
                            return;
                        }
                        hasDecimal = true;
                    }
                    number.push_back(ch.Pop());
                }
                e.vault_.d = std::stod(number);
            }
            else {
                ThrowInvalidIdentifier(ch);
            }
        }
        catch (...) {
            ThrowInvalidIdentifier(ch);
        }
    }

    void ParseStringLiteral(ChWrapper& ch, const char* target) const {
        while (*target != '\0') {
            EnsureChar(ch, *target, true);
            target++;
        }
    }

protected:
    void ParseValue(ChWrapper& ch, Entity& e) const {
        SkipWhitespace(ch);
        switch (ch.Peek())
        {
        case ('{'):
            ParseObject(ch, e);
            break;
        case ('['):
            ParseArray(ch, e);
            break;
        case ('"'):
            ParseString(ch, e);
            break;
        case ('t'):
            ParseTrue(ch, e);
            break;
        case ('f'):
            ParseFalse(ch, e);
            break;
        case ('n'):
            ParseNull(ch, e);
            break;
        default:
            ParseNumber(ch, e);
            break;
        }
    }
};


class Document : public virtual Entity {

public:
    enum ParseResult {
        kParseNotStarted,
        kParseSuccessful,
        kParseError
    };
    ParseResult parseResult;
    ChWrapper parseMessage;

public:
    Document(const Document&) = delete;
    Document& operator = (const Document& rhs) = delete;
    Document(Document&& rhs) = delete;
    Document& operator = (Document&&) noexcept = delete;

    Document() : parseResult(kParseNotStarted) { }

    template <unsigned ParsingType>
    void GenericParse(const char* ch) {
        Flush();
        try {
            if (ParsingType == kParsingTypeRecursive)
                ParseRecursive(ch, *this);
            else
                ThrowUnsupportedOperation();
            parseResult = kParseSuccessful;
        }
        catch (const std::exception& e) {
            Flush();
            parseResult = kParseError;
            parseMessage = e.what();
        }
    }

    void Parse(const char* ch) {
        GenericParse<kParsingTypeRecursive>(ch);
    }

private:
    void Flush() {
        Entity::Flush();
        parseMessage.Flush();
    }

    void ParseRecursive(ChWrapper ch, Entity& e) {
        ParseValue(ch, e);
        if (!IsEndOfStream(ch)) {
            ThrowInvalidIdentifier(ch);
        }
    }
};

JSPX_NAMESPACE_END

#endif // !JSPX_DOCUMENT_H
