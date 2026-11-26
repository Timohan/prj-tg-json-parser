/*!
 * \file
 * \brief file prj_tg_json-parser_lib_private.h
 *
 * library's private functionality
 *
 * Copyright of Timo Hannukkala. All rights reserved.
 *
 * \author Timo Hannukkala <timohannukkala@hotmail.com>
 */

#ifndef PRJ_TG_JSON_PARSER_LIB_PRIVATE_H
#define PRJ_TG_JSON_PARSER_LIB_PRIVATE_H

#include <string>
#include "../prj_tg_json_parser_lib.h"
#include "../prj_tg_json_parser_object.h"

/*!
 * \brief PrjTgJsonParserLibPrivate
 *
 * Strict recursive descent JSON (RFC 8259) parser that builds a
 * PrjTgJsonParserObject tree.
 */
class PrjTgJsonParserLibPrivate
{
public:
    //! default maximum nesting depth of arrays/objects, protects against stack overflow
    static constexpr size_t DefaultMaxDepth = 512;

    explicit PrjTgJsonParserLibPrivate();

    PrjTgJsonParserLibPrivate(const PrjTgJsonParserLibPrivate &) = delete;
    PrjTgJsonParserLibPrivate &operator=(const PrjTgJsonParserLibPrivate &) = delete;

    PrjTgJsonParserObject *getObject();
    bool parse(const std::string &filename);
    bool parseData(const std::string &data);

    PrjTgJsonParserLib::ParseError getError() const;
    std::string getErrorMessage() const;
    size_t getErrorLine() const;
    size_t getErrorColumn() const;
    size_t getErrorOffset() const;

    void setMaxDepth(size_t maxDepth);
    size_t getMaxDepth() const;
    void setMaxInputSize(size_t maxInputSize);
    size_t getMaxInputSize() const;

private:
    PrjTgJsonParserObject m_rootObject;
    size_t m_maxDepth;
    size_t m_maxInputSize;

    // state of the current parse
    const char *m_jsonStart;
    const char *m_jsonEnd;
    size_t m_bomLength;

    // last error
    PrjTgJsonParserLib::ParseError m_error;
    std::string m_errorMessage;
    size_t m_errorOffset;
    size_t m_errorLine;
    size_t m_errorColumn;

    void resetError();
    void setError(PrjTgJsonParserLib::ParseError error, const std::string &message);
    //! records syntax error at position p, returns nullptr
    const char *fail(const char *p, const char *message, PrjTgJsonParserLib::ParseError error = PrjTgJsonParserLib::ParseError_Syntax);

    bool load(const std::string &filename, std::string &data, PrjTgJsonParserLib::ParseError &error) const;
    void setTooLargeError();
    bool parseDocument(const char *jsonData, const char *jsonEnd);

    const char *skipWhitespace(const char *p) const;
    const char *parseValue(PrjTgJsonParserObject *object, const char *p, size_t depth);
    const char *parseScalar(PrjTgJsonParserObject *object, const char *p);
    const char *parseObject(PrjTgJsonParserObject *object, const char *p, size_t depth);
    const char *parseArray(PrjTgJsonParserObject *object, const char *p, size_t depth);
    const char *parseString(const char *p, std::string &text);
    const char *parseNumber(const char *p, std::string &number);
    const char *parseLiteral(const char *p, const char *literal);
};

#endif // PRJ_TG_JSON_PARSER_LIB_PRIVATE_H
