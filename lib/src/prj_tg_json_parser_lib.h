/*!
 * \file
 * \brief file prj_tg_json_parser_lib.h
 *
 * json parser library's public functionality
 *
 * Copyright of Timo Hannukkala. All rights reserved.
 *
 * \author Timo Hannukkala <timohannukkala@hotmail.com>
 */

#ifndef PRJ_TG_JSON_PARSER_LIB_H
#define PRJ_TG_JSON_PARSER_LIB_H

#include <string>
#include "prj_tg_json_parser_object.h"

/**
 * @brief Strict JSON (RFC 8259) parser
 *
 * parse() and parseData() return false if the input is not valid json,
 * in that case getObject() returns an empty root object and
 * getError()/getErrorMessage()/getErrorLine()/getErrorColumn() tell why.
 * The parser can't be copied.
 */
class TG_JSON_PARSER_LIBRARY_EXPORT PrjTgJsonParserLib
{
public:
    enum ParseError {
        ParseError_None = 0,
        ParseError_FileOpenFailed,
        ParseError_FileReadFailed,
        ParseError_Syntax,          //!< invalid json, see getErrorMessage()
        ParseError_TooDeep,         //!< arrays/objects nested deeper than getMaxDepth()
        ParseError_OutOfMemory,
        ParseError_TooLarge,        //!< input is bigger than getMaxInputSize()
    };

    explicit PrjTgJsonParserLib();
    ~PrjTgJsonParserLib();

    PrjTgJsonParserLib(const PrjTgJsonParserLib &) = delete;
    PrjTgJsonParserLib &operator=(const PrjTgJsonParserLib &) = delete;

    //! parse json file
    bool parse(const std::string &filename);
    //! parse json from memory
    bool parseData(const std::string &data);
    //! root object, owned by this parser and valid until next parse or destruction
    PrjTgJsonParserObject *getObject();

    //! error of the last parse, ParseError_None if it succeeded
    ParseError getError() const;
    //! human readable description of the last error, empty if none
    std::string getErrorMessage() const;
    /*!
     * \brief 1-based line of the last error (LF, CRLF and CR line ends)
     *
     * Set for ParseError_Syntax and ParseError_TooDeep, 0 for other errors.
     */
    size_t getErrorLine() const;
    //! 1-based column (in bytes, a BOM is not counted) of the last error, 0 if getErrorLine() is 0
    size_t getErrorColumn() const;
    //! 0-based byte offset of the last error (counted from the start of the input, including a BOM), 0 if getErrorLine() is 0
    size_t getErrorOffset() const;

    //! default maximum nesting depth of arrays and objects (512)
    static size_t getDefaultMaxDepth();
    /*!
     * \brief set maximum nesting depth of arrays and objects
     *
     * Deeper input fails with ParseError_TooDeep. Parsing and freeing the
     * tree are recursive, so the calling thread's stack must be big enough:
     * about 64 bytes per nesting level plus about 8 KB in optimized (-O2)
     * builds, several times more in debug or sanitizer builds.
     * The default 512 needs about 40 KB and fits in a 48 KB stack.
     * 0 means the default.
     */
    void setMaxDepth(size_t maxDepth);
    size_t getMaxDepth() const;

    /*!
     * \brief set maximum input size in bytes, 0 means no limit (default)
     *
     * Bigger input fails with ParseError_TooLarge. parse() stops reading
     * the file as soon as the limit is exceeded, so this also protects
     * against huge or endless files (e.g. /dev/zero).
     * The parsed tree needs several times more memory than the input.
     */
    void setMaxInputSize(size_t maxInputSize);
    size_t getMaxInputSize() const;

private:
    PrjTgJsonParserLibPrivate *m_private;
};

#endif // PRJ_TG_JSON_PARSER_LIB_H
