#ifndef JSPX_DOCUMENT_H
#define JSPX_DOCUMENT_H

#include "jspx.h"
#include "error.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <cctype>

JSPX_NAMESPACE_BEGIN

class Entity;
typedef typename std::unique_ptr<Entity> EntitySP;
typedef typename std::unordered_map<std::string, EntitySP>::const_iterator ObjectIterator;
typedef typename std::vector<EntitySP>::const_iterator ArrayIterator;


class Entity : public virtual Throwable {

private:
    enum class Identifier {
        kOpenSquareBracket = '[',
        kCloseSquareBracket = ']',
        kOpenCurlyBracket = '{',
        kCloseCurlyBracket = '}',
        kColon = ':',
        kComma = ',',
        kQuote = '"',
    };

    enum class Type {
        kTypeObject,
        kTypeArray,
        kTypeString,
        kTypeNumber,
        kTypeBoolean,
        kTypeNull,
        kTypeEmpty
    };

    struct Vault {
        double d;
        bool b;
        std::string s;

        void Flush() {
            s.clear();
            d = 0;
            b = false;
        }
        bool operator ==(const Vault& rhs) const {
            return (s == rhs.s) && (d == rhs.d) && (b == rhs.b);
        }
    } vault_;

    Type type_;
    std::unordered_map<std::string, EntitySP> o_members_;
    std::vector<EntitySP> a_members_;

public:
    Entity() : type_(Type::kTypeEmpty) { }

    Entity(const Entity& rhs) noexcept = delete;
    Entity& operator = (const Entity& rhs) = delete;

    Entity& operator = (Entity&& rhs) noexcept {
        Move(*this, rhs);
        return *this;
    };

    Entity(Entity&& rhs) noexcept {
        Move(*this, rhs);
    };

    bool operator ==(const Entity& rhs) const {
        if (type_ != rhs.type_) {
            return false;
        }
        switch (type_)
        {
        case Type::kTypeObject:
            return o_members_ == rhs.o_members_;
        case Type::kTypeArray:
            return a_members_ == rhs.a_members_;
        default:
            return vault_ == rhs.vault_;
        }
    }
    
    bool IsObject() const {
        return type_ == Type::kTypeObject;
    }

    bool IsArray() const {
        return type_ == Type::kTypeArray;
    }

    bool IsString() const {
        return type_ == Type::kTypeString;
    }

    bool IsNumber() const {
        return type_ == Type::kTypeNumber;
    }

    bool IsBoolean() const {
        return type_ == Type::kTypeBoolean;
    }

    bool IsNull() const {
        return type_ == Type::kTypeNull;
    }

    const std::string& GetString() const {
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

    bool HasMember(const std::string& key) const {
        return o_members_.count(key) > 0;
    }

    const Entity& operator [](const std::string& key) const {
        if (!IsObject()) {
            ThrowInvalidMethodCall();
        }
        return *o_members_.at(key).get();
    }

    const Entity& operator [](size_t idx) const {
        if (!IsArray()) {
            ThrowInvalidMethodCall();
        }
        return *a_members_.at(idx).get();
    }

    ArrayIterator ABegin() const {
        if (!IsArray()) {
            ThrowInvalidMethodCall();
        }
        return a_members_.begin();
    }

    ArrayIterator AEnd() const {
        if (!IsArray()) {
            ThrowInvalidMethodCall();
        }
        return a_members_.end();
    }

    ObjectIterator OBegin() const {
        if (!IsObject()) {
            ThrowInvalidMethodCall();
        }
        return o_members_.begin();
    }

    ObjectIterator OEnd() const {
        if (!IsObject()) {
            ThrowInvalidMethodCall();
        }
        return o_members_.end();
    }


private:
    void Move(Entity& target, Entity& source) {
        if (target == source) {
            return;
        }
        target.Flush();
        target.o_members_ = std::move(source.o_members_);
        target.a_members_ = std::move(source.a_members_);
        target.type_ = source.type_;
        target.vault_ = source.vault_;
        source.Flush();
    }

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
        e.SetType(Type::kTypeBoolean);
        e.vault_.b = true;
    }

    void ParseFalse(ChWrapper& ch, Entity& e) const {
        ParseStringLiteral(ch, "false");

        e.SetType(Type::kTypeBoolean);
        e.vault_.b = false;
    }

    void ParseNull(ChWrapper& ch, Entity& e) const {
        ParseStringLiteral(ch, "null");
        e.SetType(Type::kTypeNull);
    }

    void ParseString(ChWrapper& ch, Entity& e) const {
        EnsureChar(ch, Identifier::kQuote, true);
        std::string word;

        while (!CompareIdentifier(ch, Identifier::kQuote, true)) {
            if (ch.Peek() == '\0')
                ThrowInvalidIdentifier(ch);
            word.push_back(ch.Pop());
        }

        e.SetType(Type::kTypeString);
        e.vault_.s = std::move(word);
    }

    void ParseObject(ChWrapper& ch, Entity& e) const {
        e.SetType(Type::kTypeObject);
        EnsureChar(ch, Identifier::kOpenCurlyBracket, true);
        SkipWhitespace(ch);

        if (CompareIdentifier(ch, Identifier::kCloseCurlyBracket, true))
            return;
        SkipWhitespace(ch);

        while (true) {

            Entity key;
            ParseString(ch, key);
            SkipWhitespace(ch);

            EnsureChar(ch, Identifier::kColon, true);
            SkipWhitespace(ch);

            Entity value;
            ParseValue(ch, value);
            SkipWhitespace(ch);

            e.o_members_.insert_or_assign(
                key.GetString(),
                std::make_unique<Entity>(std::move(value))
            );

            if (CompareIdentifier(ch, Identifier::kCloseCurlyBracket, true))
                break;

            EnsureChar(ch, Identifier::kComma, true);
            SkipWhitespace(ch);
        }
    }

    void ParseArray(ChWrapper& ch, Entity& e) const {
        e.SetType(Type::kTypeArray);
        EnsureChar(ch, Identifier::kOpenSquareBracket, true);
        SkipWhitespace(ch);
        if (CompareIdentifier(ch, Identifier::kCloseSquareBracket, true))
            return;
        SkipWhitespace(ch);

        while (true) {

            Entity member;
            ParseValue(ch, member);
            SkipWhitespace(ch);
            e.a_members_.push_back(std::make_unique<Entity>(std::move(member)));

            SkipWhitespace(ch);
            if (CompareIdentifier(ch, Identifier::kCloseSquareBracket, true))
                break;

            EnsureChar(ch, Identifier::kComma, true);
            SkipWhitespace(ch);
        }
    }

    void ParseNumber(ChWrapper& ch, Entity& e) const {
        SkipWhitespace(ch);
        try {
            if (CheckChar(ch, '-') || (std::isdigit(ch.Peek()) && !CheckChar(ch, '0')))
            {
                e.SetType(Type::kTypeNumber);

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
    
    void Flush() {
        SetType(Type::kTypeEmpty);
        o_members_.clear();
        a_members_.clear();
        vault_.Flush();
    }
};


class Document : public virtual Entity {

public:
    enum class ParseResult {
        kParseNotStarted,
        kParseSuccessful,
        kParseError
    };

    enum class ParsingType {
        kParsingTypeIterative,
        kParsingTypeRecursive
    };
    ParseResult parseResult;
    ChWrapper parseMessage;

public:
    Document(const Document&) = delete;
    Document& operator = (const Document& rhs) = delete;
    Document(Document&& rhs) = delete;
    Document& operator = (Document&&) noexcept = delete;

    Document() : parseResult(ParseResult::kParseNotStarted) { }

    template <ParsingType ParsingType>
    void GenericParse(const char* ch) {
        Flush();
        try {
            if (ParsingType == ParsingType::kParsingTypeRecursive)
                ParseRecursive(ch, *this);
            else
                ThrowUnsupportedOperation();
            parseResult = ParseResult::kParseSuccessful;
        }
        catch (const std::exception& e) {
            Flush();
            parseResult = ParseResult::kParseError;
            parseMessage = e.what();
        }
    }

    void Parse(const char* ch) {
        GenericParse<ParsingType::kParsingTypeRecursive>(ch);
    }

private:
    void Flush() {
        Entity::Flush();
        parseMessage.Flush();
    }

    void ParseRecursive(ChWrapper ch, Entity& e) {
        ParseValue(ch, e);
        if (ch.Peek() != '\0') {
            ThrowInvalidIdentifier(ch);
        }
    }
};

JSPX_NAMESPACE_END

#endif // !JSPX_DOCUMENT_H
