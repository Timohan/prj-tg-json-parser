#include "prj_tg_json_parser_lib_private.h"
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <new>

// keeps the function out of its callers, see parseValue()
#if defined(_MSC_VER)
#define TG_JSON_NOINLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
#define TG_JSON_NOINLINE __attribute__((noinline))
#else
#define TG_JSON_NOINLINE
#endif

namespace {

bool parseHex4(const char *data, const char *end, uint32_t &value)
{
    if (end - data < 4) {
        return false;
    }
    value = 0;
    for (int i=0;i<4;i++) {
        const char c = data[i];
        uint32_t digit;
        if (c >= '0' && c <= '9') {
            digit = static_cast<uint32_t>(c - '0');
        } else if (c >= 'a' && c <= 'f') {
            digit = static_cast<uint32_t>(c - 'a' + 10);
        } else if (c >= 'A' && c <= 'F') {
            digit = static_cast<uint32_t>(c - 'A' + 10);
        } else {
            return false;
        }
        value = (value << 4) | digit;
    }
    return true;
}

void appendUtf8(uint32_t codePoint, std::string &text)
{
    if (codePoint < 0x80) {
        text.push_back(static_cast<char>(codePoint));
    } else if (codePoint < 0x800) {
        text.push_back(static_cast<char>(0xC0 | (codePoint >> 6)));
        text.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
    } else if (codePoint < 0x10000) {
        text.push_back(static_cast<char>(0xE0 | (codePoint >> 12)));
        text.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
        text.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
    } else {
        text.push_back(static_cast<char>(0xF0 | (codePoint >> 18)));
        text.push_back(static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F)));
        text.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
        text.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
    }
}

/*!
 * \brief length of the valid UTF-8 multibyte sequence at p, 0 if invalid
 *
 * Rejects overlong encodings, surrogates and code points above U+10FFFF.
 */
size_t utf8SequenceLength(const char *p, const char *end)
{
    const unsigned char lead = static_cast<unsigned char>(*p);
    size_t length;
    uint32_t codePoint;
    if (lead >= 0xC2 && lead <= 0xDF) {
        length = 2;
        codePoint = lead & 0x1F;
    } else if (lead >= 0xE0 && lead <= 0xEF) {
        length = 3;
        codePoint = lead & 0x0F;
    } else if (lead >= 0xF0 && lead <= 0xF4) {
        length = 4;
        codePoint = lead & 0x07;
    } else {
        return 0;
    }
    if (static_cast<size_t>(end - p) < length) {
        return 0;
    }
    for (size_t i=1;i<length;i++) {
        const unsigned char c = static_cast<unsigned char>(p[i]);
        if ((c & 0xC0) != 0x80) {
            return 0;
        }
        codePoint = (codePoint << 6) | (c & 0x3F);
    }
    if ((length == 3 && codePoint < 0x800)
        || (length == 4 && (codePoint < 0x10000 || codePoint > 0x10FFFF))
        || (codePoint >= 0xD800 && codePoint <= 0xDFFF)) {
        return 0;
    }
    return length;
}

bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}

} // namespace

PrjTgJsonParserLibPrivate::PrjTgJsonParserLibPrivate()
    : m_rootObject(nullptr)
    , m_maxDepth(DefaultMaxDepth)
    , m_maxInputSize(0)
    , m_jsonStart(nullptr)
    , m_jsonEnd(nullptr)
    , m_bomLength(0)
    , m_error(PrjTgJsonParserLib::ParseError_None)
    , m_errorOffset(0)
    , m_errorLine(0)
    , m_errorColumn(0)
{
}

PrjTgJsonParserObject *PrjTgJsonParserLibPrivate::getObject()
{
    return &m_rootObject;
}

PrjTgJsonParserLib::ParseError PrjTgJsonParserLibPrivate::getError() const
{
    return m_error;
}

std::string PrjTgJsonParserLibPrivate::getErrorMessage() const
{
    return m_errorMessage;
}

size_t PrjTgJsonParserLibPrivate::getErrorLine() const
{
    return m_errorLine;
}

size_t PrjTgJsonParserLibPrivate::getErrorColumn() const
{
    return m_errorColumn;
}

size_t PrjTgJsonParserLibPrivate::getErrorOffset() const
{
    return m_errorOffset;
}

void PrjTgJsonParserLibPrivate::setMaxDepth(size_t maxDepth)
{
    m_maxDepth = maxDepth == 0 ? DefaultMaxDepth : maxDepth;
}

size_t PrjTgJsonParserLibPrivate::getMaxDepth() const
{
    return m_maxDepth;
}

void PrjTgJsonParserLibPrivate::setMaxInputSize(size_t maxInputSize)
{
    m_maxInputSize = maxInputSize;
}

size_t PrjTgJsonParserLibPrivate::getMaxInputSize() const
{
    return m_maxInputSize;
}

void PrjTgJsonParserLibPrivate::setTooLargeError()
{
    m_rootObject.clear();
    resetError();
    setError(PrjTgJsonParserLib::ParseError_TooLarge,
             "Input is larger than " + std::to_string(m_maxInputSize) + " bytes");
}

void PrjTgJsonParserLibPrivate::resetError()
{
    m_error = PrjTgJsonParserLib::ParseError_None;
    m_errorMessage.clear();
    m_errorOffset = 0;
    m_errorLine = 0;
    m_errorColumn = 0;
}

void PrjTgJsonParserLibPrivate::setError(PrjTgJsonParserLib::ParseError error, const std::string &message)
{
    m_error = error;
    m_errorMessage = message;
}

const char *PrjTgJsonParserLibPrivate::fail(const char *p, const char *message, PrjTgJsonParserLib::ParseError error)
{
    m_error = error;
    m_errorOffset = static_cast<size_t>(p - m_jsonStart);
    // line ends: LF, CRLF and CR; the BOM is not part of the first line's columns
    m_errorLine = 1;
    m_errorColumn = 1;
    for (const char *c = m_jsonStart+m_bomLength; c < p; ++c) {
        if (*c == '\n' || (*c == '\r' && (c+1 >= m_jsonEnd || c[1] != '\n'))) {
            ++m_errorLine;
            m_errorColumn = 1;
        } else if (*c != '\r') {
            ++m_errorColumn;
        }
    }
    m_errorMessage = std::string(message) + " at line " + std::to_string(m_errorLine)
                     + " column " + std::to_string(m_errorColumn);
    return nullptr;
}

bool PrjTgJsonParserLibPrivate::parseData(const std::string &data)
{
    if (m_maxInputSize != 0 && data.size() > m_maxInputSize) {
        setTooLargeError();
        return false;
    }
    return parseDocument(data.data(), data.data()+data.size());
}

bool PrjTgJsonParserLibPrivate::parse(const std::string &filename)
{
    std::string data;
    PrjTgJsonParserLib::ParseError error;
    if (!load(filename, data, error)) {
        if (error == PrjTgJsonParserLib::ParseError_TooLarge) {
            setTooLargeError();
            return false;
        }
        m_rootObject.clear();
        resetError();
        switch (error) {
            case PrjTgJsonParserLib::ParseError_FileOpenFailed:
                setError(error, "Can't open file: " + filename);
                break;
            case PrjTgJsonParserLib::ParseError_OutOfMemory:
                setError(error, "Out of memory while reading file: " + filename);
                break;
            case PrjTgJsonParserLib::ParseError_None:
            case PrjTgJsonParserLib::ParseError_FileReadFailed:
            case PrjTgJsonParserLib::ParseError_Syntax:
            case PrjTgJsonParserLib::ParseError_TooDeep:
            case PrjTgJsonParserLib::ParseError_TooLarge:
            default:
                setError(PrjTgJsonParserLib::ParseError_FileReadFailed, "Can't read file: " + filename);
                break;
        }
        return false;
    }
    return parseDocument(data.data(), data.data()+data.size());
}

bool PrjTgJsonParserLibPrivate::load(const std::string &filename, std::string &data, PrjTgJsonParserLib::ParseError &error) const
{
    FILE *fp = fopen(filename.c_str(), "rb");
    if (!fp) {
        error = PrjTgJsonParserLib::ParseError_FileOpenFailed;
        return false;
    }
    char buffer[65536];
    size_t count;
    try {
        while ((count = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
            if (m_maxInputSize != 0 && count > m_maxInputSize - data.size()) {
                fclose(fp);
                error = PrjTgJsonParserLib::ParseError_TooLarge;
                return false;
            }
            data.append(buffer, count);
        }
    } catch (const std::bad_alloc &) {
        fclose(fp);
        error = PrjTgJsonParserLib::ParseError_OutOfMemory;
        return false;
    }
    const bool ok = !ferror(fp);
    fclose(fp);
    if (!ok) {
        error = PrjTgJsonParserLib::ParseError_FileReadFailed;
    }
    return ok;
}

/*!
 * \brief parse whole document, on failure the root object is left empty
 */
bool PrjTgJsonParserLibPrivate::parseDocument(const char *jsonData, const char *jsonEnd)
{
    m_rootObject.clear();
    resetError();
    m_jsonStart = jsonData;
    m_jsonEnd = jsonEnd;

    // skip UTF-8 byte order mark
    m_bomLength = 0;
    if (jsonEnd - jsonData >= 3 && memcmp(jsonData, "\xEF\xBB\xBF", 3) == 0) {
        m_bomLength = 3;
    }
    const char *p = jsonData+m_bomLength;
    try {
        p = skipWhitespace(p);
        if (p >= m_jsonEnd) {
            p = fail(p, "Empty document");
        } else {
            p = parseValue(&m_rootObject, p, 0);
        }
        if (p) {
            p = skipWhitespace(p);
            if (p != m_jsonEnd) {
                p = fail(p, "Unexpected data after json value");
            }
        }
    } catch (const std::bad_alloc &) {
        m_rootObject.clear();
        resetError();
        setError(PrjTgJsonParserLib::ParseError_OutOfMemory, "Out of memory");
        p = nullptr;
    }
    m_jsonStart = nullptr;
    m_jsonEnd = nullptr;
    m_bomLength = 0;
    if (!p) {
        m_rootObject.clear();
        return false;
    }
    return true;
}

const char *PrjTgJsonParserLibPrivate::skipWhitespace(const char *p) const
{
    while (p < m_jsonEnd && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) {
        ++p;
    }
    return p;
}

/*!
 * \brief parse one json value starting exactly at p
 *
 * This is on the recursion path, so it has no locals: scalars are parsed in
 * parseScalar() which is never inlined here, keeping the stack use per
 * nesting level small.
 * \return position after the value, nullptr on error
 */
const char *PrjTgJsonParserLibPrivate::parseValue(PrjTgJsonParserObject *object, const char *p, size_t depth)
{
    if (p >= m_jsonEnd) {
        return fail(p, "Unexpected end of data, expected value");
    }
    if (*p == '{') {
        return parseObject(object, p, depth);
    }
    if (*p == '[') {
        return parseArray(object, p, depth);
    }
    return parseScalar(object, p);
}

/*!
 * \brief parse string, number, true, false or null starting exactly at p
 * \return position after the value, nullptr on error
 */
TG_JSON_NOINLINE
const char *PrjTgJsonParserLibPrivate::parseScalar(PrjTgJsonParserObject *object, const char *p)
{
    const char *next;
    std::string text;
    switch (*p) {
        case '\"':
            next = parseString(p, text);
            if (next) {
                object->setType(PrjTgJsonParserObject::PrjTgJsonParserObjectType_Text);
                object->setText(std::move(text));
            }
            return next;
        case 't':
        case 'f':
            next = parseLiteral(p, *p == 't' ? "true" : "false");
            if (next) {
                object->setType(PrjTgJsonParserObject::PrjTgJsonParserObjectType_Boolean);
                object->setText(*p == 't' ? "true" : "false");
            }
            return next;
        case 'n':
            next = parseLiteral(p, "null");
            if (next) {
                object->setNull();
            }
            return next;
        default:
            if (*p != '-' && !isDigit(*p)) {
                return fail(p, "Unexpected character, expected value");
            }
            next = parseNumber(p, text);
            if (next) {
                object->setType(PrjTgJsonParserObject::PrjTgJsonParserObjectType_Value);
                object->setText(std::move(text));
            }
            return next;
    }
}

const char *PrjTgJsonParserLibPrivate::parseObject(PrjTgJsonParserObject *object, const char *p, size_t depth)
{
    if (depth >= m_maxDepth) {
        return fail(p, "Too deep nesting", PrjTgJsonParserLib::ParseError_TooDeep);
    }
    object->setType(PrjTgJsonParserObject::PrjTgJsonParserObjectType_Array);
    object->setJsonObject(true);

    p = skipWhitespace(p+1);
    if (p < m_jsonEnd && *p == '}') {
        return p+1;
    }
    while (1) {
        if (p >= m_jsonEnd || *p != '\"') {
            return fail(p, "Expected member name");
        }
        // name is parsed directly into the child, so no string is kept on the recursion stack
        PrjTgJsonParserObject *newObject = object->addChild();
        p = parseString(p, newObject->m_name);
        if (!p) {
            return nullptr;
        }
        p = skipWhitespace(p);
        if (p >= m_jsonEnd || *p != ':') {
            return fail(p, "Expected ':'");
        }
        p = skipWhitespace(p+1);

        p = parseValue(newObject, p, depth+1);
        if (!p) {
            return nullptr;
        }

        p = skipWhitespace(p);
        if (p < m_jsonEnd && *p == '}') {
            return p+1;
        }
        if (p >= m_jsonEnd || *p != ',') {
            return fail(p, "Expected ',' or '}'");
        }
        p = skipWhitespace(p+1);
    }
}

const char *PrjTgJsonParserLibPrivate::parseArray(PrjTgJsonParserObject *object, const char *p, size_t depth)
{
    if (depth >= m_maxDepth) {
        return fail(p, "Too deep nesting", PrjTgJsonParserLib::ParseError_TooDeep);
    }
    object->setType(PrjTgJsonParserObject::PrjTgJsonParserObjectType_Array);
    object->setJsonObject(false);

    p = skipWhitespace(p+1);
    if (p < m_jsonEnd && *p == ']') {
        return p+1;
    }
    while (1) {
        PrjTgJsonParserObject *newObject = object->addChild();
        p = parseValue(newObject, p, depth+1);
        if (!p) {
            return nullptr;
        }

        p = skipWhitespace(p);
        if (p < m_jsonEnd && *p == ']') {
            return p+1;
        }
        if (p >= m_jsonEnd || *p != ',') {
            return fail(p, "Expected ',' or ']'");
        }
        p = skipWhitespace(p+1);
    }
}

/*!
 * \brief parse json string starting at the opening quote and decode escapes
 * \param text [out] decoded text in UTF-8
 * \return position after the closing quote, nullptr on error
 */
const char *PrjTgJsonParserLibPrivate::parseString(const char *p, std::string &text)
{
    text.clear();
    const char *start = p;
    ++p;
    while (1) {
        if (p >= m_jsonEnd) {
            return fail(start, "Unterminated string");
        }
        const char c = *p;
        if (c == '\"') {
            return p+1;
        }
        const unsigned char uc = static_cast<unsigned char>(c);
        if (uc < 0x20) {
            return fail(p, "Control character in string must be escaped");
        }
        if (uc >= 0x80) {
            const size_t length = utf8SequenceLength(p, m_jsonEnd);
            if (length == 0) {
                return fail(p, "Invalid UTF-8 in string");
            }
            text.append(p, length);
            p += length;
            continue;
        }
        if (c != '\\') {
            text.push_back(c);
            ++p;
            continue;
        }
        const char *escape = p;
        ++p;
        if (p >= m_jsonEnd) {
            return fail(start, "Unterminated string");
        }
        switch (*p) {
            case '\"': text.push_back('\"'); break;
            case '\\': text.push_back('\\'); break;
            case '/': text.push_back('/'); break;
            case 'b': text.push_back('\b'); break;
            case 'f': text.push_back('\f'); break;
            case 'n': text.push_back('\n'); break;
            case 'r': text.push_back('\r'); break;
            case 't': text.push_back('\t'); break;
            case 'u': {
                uint32_t codePoint;
                if (!parseHex4(p+1, m_jsonEnd, codePoint)) {
                    return fail(escape, "Invalid \\u escape");
                }
                p += 4;
                if (codePoint >= 0xD800 && codePoint <= 0xDBFF) {
                    // high surrogate, must be followed by \uDC00-\uDFFF
                    uint32_t low;
                    if (m_jsonEnd - p < 3 || p[1] != '\\' || p[2] != 'u'
                        || !parseHex4(p+3, m_jsonEnd, low)
                        || low < 0xDC00 || low > 0xDFFF) {
                        return fail(escape, "Unpaired UTF-16 surrogate in \\u escape");
                    }
                    p += 6;
                    codePoint = 0x10000 + ((codePoint - 0xD800) << 10) + (low - 0xDC00);
                } else if (codePoint >= 0xDC00 && codePoint <= 0xDFFF) {
                    return fail(escape, "Unpaired UTF-16 surrogate in \\u escape");
                }
                appendUtf8(codePoint, text);
                break;
            }
            default:
                return fail(escape, "Invalid escape in string");
        }
        ++p;
    }
}

/*!
 * \brief parse json number: -?(0|[1-9][0-9]*)(.[0-9]+)?([eE][+-]?[0-9]+)?
 * \param number [out] number text
 * \return position after the number, nullptr on error
 */
const char *PrjTgJsonParserLibPrivate::parseNumber(const char *p, std::string &number)
{
    const char *start = p;
    if (p < m_jsonEnd && *p == '-') {
        ++p;
    }
    if (p >= m_jsonEnd || !isDigit(*p)) {
        return fail(start, "Invalid number");
    }
    if (*p == '0') {
        ++p;
    } else {
        while (p < m_jsonEnd && isDigit(*p)) {
            ++p;
        }
    }
    if (p < m_jsonEnd && *p == '.') {
        ++p;
        if (p >= m_jsonEnd || !isDigit(*p)) {
            return fail(start, "Invalid number");
        }
        while (p < m_jsonEnd && isDigit(*p)) {
            ++p;
        }
    }
    if (p < m_jsonEnd && (*p == 'e' || *p == 'E')) {
        ++p;
        if (p < m_jsonEnd && (*p == '+' || *p == '-')) {
            ++p;
        }
        if (p >= m_jsonEnd || !isDigit(*p)) {
            return fail(start, "Invalid number");
        }
        while (p < m_jsonEnd && isDigit(*p)) {
            ++p;
        }
    }
    number.assign(start, p);
    return p;
}

const char *PrjTgJsonParserLibPrivate::parseLiteral(const char *p, const char *literal)
{
    const size_t length = strlen(literal);
    if (static_cast<size_t>(m_jsonEnd - p) < length || memcmp(p, literal, length) != 0) {
        return fail(p, "Invalid literal");
    }
    return p+length;
}
