/*!
 * \file
 * \brief file prj_tg_json_parser_test.h
 *
 * PrjTgJsonParserLib library's test class
 *
 * Copyright of Timo Hannukkala. All rights reserved.
 *
 * \author Timo Hannukkala <timohannukkala@hotmail.com>
 */

#ifndef PRJ_TG_JSON_PARSER_TEST_H
#define PRJ_TG_JSON_PARSER_TEST_H

#include <string>

class PrjTgJsonParserObject;
class PrjTgJsonParserLib;

class PrjTgHJsonParserTest
{
public:
    bool test(const std::string &ordersFileName);

private:
    static void removeNonRequiredCharacters(std::string &line);
    static bool testParseFails(PrjTgJsonParserLib &tgJsonParserLib, const std::string &argument, int lineNumber);
    static bool parseUnsigned(const std::string &text, size_t &value, int lineNumber);
    static bool runCommand(const std::string &command, const std::string &argument,
                           PrjTgJsonParserObject *&currentObject, int lineNumber);

};

#endif 
