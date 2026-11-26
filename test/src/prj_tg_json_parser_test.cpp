/*!
 * \file
 * \brief file prj_tg_json_parser_test.cpp
 *
 * PrjTgJsonParserLib library's test class
 *
 * Orders file format, one command per line, "#" starts a comment line:
 *   File <json file>                  parse file, must succeed, current object = root
 *   maxDepth <depth>                  set parser's maximum nesting depth (0 = default)
 *   maxInputSize <bytes>              set parser's maximum input size (0 = no limit)
 *   parseFails <json file> [<error> [<line> <column>]]
 *                                     parse file, must fail; error is one of
 *                                     FileOpenFailed, FileReadFailed, Syntax, TooDeep, OutOfMemory, TooLarge
 *   upObject <index>                  go to child by index
 *   upObjectName <name>               go to child by name
 *   noObjectName <name>               getObject(name) must return null
 *   downObject                        go to parent
 *   isObjectCount <count>
 *   isName <name>                     (empty name: "isName")
 *   isText <text>                     (empty text: "isText")
 *   isValue <number>
 *   valueFails                        value object whose getValue(double&) fails
 *   isNumberText <text>
 *   isInt64 <integer>
 *   int64Fails                        value object whose getInt64() fails
 *   isBoolean true|false
 *   isArray / isObject / isNull
 * Json file paths are relative to the orders folder unless they start with "/".
 * Arguments are used as is, including trailing spaces.
 *
 * Copyright of Timo Hannukkala. All rights reserved.
 *
 * \author Timo Hannukkala <timohannukkala@hotmail.com>
 */

#include "prj_tg_json_parser_test.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>
#include <prj_tg_json_parser_lib.h>
#include <prj_tg_json_parser_object.h>
#include "tg_global_log.h"

namespace {

std::string resolveFileName(const std::string &name)
{
    if (!name.empty() && name.front() == '/') {
        return name;
    }
    return std::string(ORDERS_FOLDER) + "/" + name;
}

const char *errorName(PrjTgJsonParserLib::ParseError error)
{
    switch (error) {
        case PrjTgJsonParserLib::ParseError_None: return "None";
        case PrjTgJsonParserLib::ParseError_FileOpenFailed: return "FileOpenFailed";
        case PrjTgJsonParserLib::ParseError_FileReadFailed: return "FileReadFailed";
        case PrjTgJsonParserLib::ParseError_Syntax: return "Syntax";
        case PrjTgJsonParserLib::ParseError_TooDeep: return "TooDeep";
        case PrjTgJsonParserLib::ParseError_OutOfMemory: return "OutOfMemory";
        case PrjTgJsonParserLib::ParseError_TooLarge: return "TooLarge";
        default: return "Unknown";
    }
}

} // namespace

bool PrjTgHJsonParserTest::test(const std::string &ordersFileName)
{
    std::ifstream file(ordersFileName);
    if (!file.is_open()) {
        TG_ERROR_LOG("Can't open orders file:", ordersFileName);
        return false;
    }
    std::string line;
    PrjTgJsonParserLib tgJsonParserLib;
    PrjTgJsonParserObject *currentObject = nullptr;
    int lineNumber = 0;
    TG_DEBUG_LOG(ordersFileName);

    while (getline(file, line)) {
        lineNumber++;
        removeNonRequiredCharacters(line);
        if (line.empty() || line.front() == '#') {
            continue;
        }
        // split line to command and argument
        const size_t spacePos = line.find(' ');
        const std::string command = line.substr(0, spacePos);
        const std::string argument = spacePos == std::string::npos ? "" : line.substr(spacePos+1);

        if (command == "File") {
            if (argument.empty()) {
                TG_ERROR_LOG("File name missing", "Orders line number: ", lineNumber);
                return false;
            }
            const std::string fileName = resolveFileName(argument);
            TG_DEBUG_LOG("Parsing start:", fileName);
            if (!tgJsonParserLib.parse(fileName)) {
                TG_ERROR_LOG("Parsing failed:", fileName, tgJsonParserLib.getErrorMessage(), "Orders line number: ", lineNumber);
                return false;
            }
            if (tgJsonParserLib.getError() != PrjTgJsonParserLib::ParseError_None
                || !tgJsonParserLib.getErrorMessage().empty()) {
                TG_ERROR_LOG("Error not reset after successful parse:", fileName, "Orders line number: ", lineNumber);
                return false;
            }
            currentObject = tgJsonParserLib.getObject();
            continue;
        }
        if (command == "maxDepth") {
            size_t depth;
            if (!parseUnsigned(argument, depth, lineNumber)) {
                return false;
            }
            tgJsonParserLib.setMaxDepth(depth);
            const size_t expected = depth == 0 ? PrjTgJsonParserLib::getDefaultMaxDepth() : depth;
            if (tgJsonParserLib.getMaxDepth() != expected) {
                TG_ERROR_LOG("getMaxDepth is incorrect:", tgJsonParserLib.getMaxDepth(), "Should be: ", expected, "Orders line number: ", lineNumber);
                return false;
            }
            continue;
        }
        if (command == "maxInputSize") {
            size_t size;
            if (!parseUnsigned(argument, size, lineNumber)) {
                return false;
            }
            tgJsonParserLib.setMaxInputSize(size);
            if (tgJsonParserLib.getMaxInputSize() != size) {
                TG_ERROR_LOG("getMaxInputSize is incorrect:", tgJsonParserLib.getMaxInputSize(), "Should be: ", size, "Orders line number: ", lineNumber);
                return false;
            }
            continue;
        }
        if (command == "parseFails") {
            if (!testParseFails(tgJsonParserLib, argument, lineNumber)) {
                return false;
            }
            currentObject = nullptr;
            continue;
        }
        if (!currentObject) {
            TG_ERROR_LOG("No parsed file, use File command first.", "Orders line number: ", lineNumber);
            return false;
        }
        if (!runCommand(command, argument, currentObject, lineNumber)) {
            return false;
        }
    }
    return true;
}

bool PrjTgHJsonParserTest::testParseFails(PrjTgJsonParserLib &tgJsonParserLib, const std::string &argument, int lineNumber)
{
    // <file> [<error> [<line> <column>]]
    std::istringstream stream(argument);
    std::string file;
    std::string expectedError;
    std::string expectedLine;
    std::string expectedColumn;
    stream >> file >> expectedError >> expectedLine >> expectedColumn;
    if (file.empty()) {
        TG_ERROR_LOG("File name missing", "Orders line number: ", lineNumber);
        return false;
    }
    const std::string fileName = resolveFileName(file);
    if (tgJsonParserLib.parse(fileName)) {
        TG_ERROR_LOG("Parsing should fail:", fileName, "Orders line number: ", lineNumber);
        return false;
    }
    if (tgJsonParserLib.getErrorMessage().empty()
        || tgJsonParserLib.getObject()->getObjectCount() != 0) {
        TG_ERROR_LOG("Failed parse must give error message and empty root:", fileName, "Orders line number: ", lineNumber);
        return false;
    }
    TG_DEBUG_LOG("Expected failure:", fileName, tgJsonParserLib.getErrorMessage());
    if (!expectedError.empty() && expectedError != errorName(tgJsonParserLib.getError())) {
        TG_ERROR_LOG("Error is incorrect:", errorName(tgJsonParserLib.getError()), "Should be: ", expectedError,
                     tgJsonParserLib.getErrorMessage(), "Orders line number: ", lineNumber);
        return false;
    }
    if (!expectedLine.empty()) {
        size_t line;
        size_t column;
        if (!parseUnsigned(expectedLine, line, lineNumber) || !parseUnsigned(expectedColumn, column, lineNumber)) {
            return false;
        }
        if (tgJsonParserLib.getErrorLine() != line || tgJsonParserLib.getErrorColumn() != column) {
            TG_ERROR_LOG("Error position is incorrect:", tgJsonParserLib.getErrorLine(), tgJsonParserLib.getErrorColumn(),
                         "Should be: ", line, column, "Orders line number: ", lineNumber);
            return false;
        }
    }
    return true;
}

bool PrjTgHJsonParserTest::parseUnsigned(const std::string &text, size_t &value, int lineNumber)
{
    try {
        size_t pos = 0;
        value = std::stoul(text, &pos);
        if (pos == text.size()) {
            return true;
        }
    } catch (...) {
    }
    TG_ERROR_LOG("Invalid number:", text, "Orders line number: ", lineNumber);
    return false;
}

bool PrjTgHJsonParserTest::runCommand(const std::string &command, const std::string &argument,
                                      PrjTgJsonParserObject *&currentObject, int lineNumber)
{
    size_t index;
    if (command == "isObjectCount") {
        if (!parseUnsigned(argument, index, lineNumber)) {
            return false;
        }
        if (currentObject->getObjectCount() != index) {
            TG_ERROR_LOG("Object count is incorrect:", currentObject->getObjectCount(), "Should be: ", index, "Orders line number: ", lineNumber);
            return false;
        }
        return true;
    }
    if (command == "upObject") {
        if (!parseUnsigned(argument, index, lineNumber)) {
            return false;
        }
        currentObject = currentObject->getObject(index);
        if (!currentObject) {
            TG_ERROR_LOG("upObject is null:", index, "Orders line number: ", lineNumber);
            return false;
        }
        return true;
    }
    if (command == "upObjectName") {
        currentObject = currentObject->getObject(argument);
        if (!currentObject) {
            TG_ERROR_LOG("upObjectName is null:", argument, "Orders line number: ", lineNumber);
            return false;
        }
        return true;
    }
    if (command == "noObjectName") {
        if (currentObject->getObject(argument)) {
            TG_ERROR_LOG("getObject(name) should be null:", argument, "Orders line number: ", lineNumber);
            return false;
        }
        return true;
    }
    if (command == "downObject") {
        currentObject = currentObject->getParent();
        if (!currentObject) {
            TG_ERROR_LOG("downObject is null", "Orders line number: ", lineNumber);
            return false;
        }
        return true;
    }
    if (command == "isValue") {
        if (currentObject->getType() != PrjTgJsonParserObject::PrjTgJsonParserObjectType_Value) {
            TG_ERROR_LOG("current object is not value:", currentObject->getType(), "Orders line number: ", lineNumber);
            return false;
        }
        double expected;
        try {
            expected = std::stod(argument);
        } catch (...) {
            TG_ERROR_LOG("Invalid value:", argument, "Orders line number: ", lineNumber);
            return false;
        }
        double value;
        if (!currentObject->getValue(value) || std::abs(expected-value) > 0.0) {
            TG_ERROR_LOG("Value is incorrect:", currentObject->getValue(), "Should be: ", expected, "Orders line number: ", lineNumber);
            return false;
        }
        return true;
    }
    if (command == "valueFails") {
        double value;
        if (currentObject->getType() != PrjTgJsonParserObject::PrjTgJsonParserObjectType_Value
            || currentObject->getValue(value)) {
            TG_ERROR_LOG("getValue should fail on value object", "Orders line number: ", lineNumber);
            return false;
        }
        return true;
    }
    if (command == "isNumberText") {
        if (currentObject->getNumberText() != argument) {
            TG_ERROR_LOG("Number text is incorrect:", currentObject->getNumberText(), "Should be: ", argument, "Orders line number: ", lineNumber);
            return false;
        }
        return true;
    }
    if (command == "isInt64") {
        int64_t expected;
        try {
            size_t pos = 0;
            expected = std::stoll(argument, &pos);
            if (pos != argument.size()) {
                throw std::invalid_argument(argument);
            }
        } catch (...) {
            TG_ERROR_LOG("Invalid integer:", argument, "Orders line number: ", lineNumber);
            return false;
        }
        int64_t value;
        if (!currentObject->getInt64(value) || value != expected) {
            TG_ERROR_LOG("Int64 is incorrect:", currentObject->getNumberText(), "Should be: ", argument, "Orders line number: ", lineNumber);
            return false;
        }
        return true;
    }
    if (command == "int64Fails") {
        int64_t value;
        if (currentObject->getType() != PrjTgJsonParserObject::PrjTgJsonParserObjectType_Value
            || currentObject->getInt64(value)) {
            TG_ERROR_LOG("getInt64 should fail on value object", "Orders line number: ", lineNumber);
            return false;
        }
        return true;
    }
    if (command == "isText") {
        if (currentObject->getType() != PrjTgJsonParserObject::PrjTgJsonParserObjectType_Text) {
            TG_ERROR_LOG("current object is not text:", currentObject->getType(), "Orders line number: ", lineNumber);
            return false;
        }
        if (argument != currentObject->getText()) {
            TG_ERROR_LOG("Text is incorrect:", currentObject->getText(), "Should be: ", argument, "Orders line number: ", lineNumber);
            return false;
        }
        return true;
    }
    if (command == "isName") {
        if (argument != currentObject->getName()) {
            TG_ERROR_LOG("Name is incorrect:", currentObject->getName(), "Should be: ", argument, "Orders line number: ", lineNumber);
            return false;
        }
        return true;
    }
    if (command == "isArray") {
        if (currentObject->getType() != PrjTgJsonParserObject::PrjTgJsonParserObjectType_Array
            || currentObject->isJsonObject()) {
            TG_ERROR_LOG("current object is not array:", currentObject->getType(), "Orders line number: ", lineNumber);
            return false;
        }
        return true;
    }
    if (command == "isObject") {
        if (!currentObject->isJsonObject()) {
            TG_ERROR_LOG("current object is not json object:", currentObject->getType(), "Orders line number: ", lineNumber);
            return false;
        }
        return true;
    }
    if (command == "isNull") {
        if (!currentObject->isNull()) {
            TG_ERROR_LOG("current object is not null:", currentObject->getType(), "Orders line number: ", lineNumber);
            return false;
        }
        return true;
    }
    if (command == "isBoolean") {
        if (currentObject->getType() != PrjTgJsonParserObject::PrjTgJsonParserObjectType_Boolean) {
            TG_ERROR_LOG("current object is not boolean:", currentObject->getType(), "Orders line number: ", lineNumber);
            return false;
        }
        if (argument != "true" && argument != "false") {
            TG_ERROR_LOG("isBoolean argument must be true or false:", argument, "Orders line number: ", lineNumber);
            return false;
        }
        if (currentObject->getBoolean() != (argument == "true")) {
            TG_ERROR_LOG("isBoolean is incorrect:", currentObject->getBoolean(), "Should be: ", argument, "Orders line number: ", lineNumber);
            return false;
        }
        return true;
    }
    TG_ERROR_LOG("Unknown command:", command, "Orders line number: ", lineNumber);
    return false;
}

void PrjTgHJsonParserTest::removeNonRequiredCharacters(std::string &line)
{
    // only line endings are removed, trailing spaces are part of the argument
    while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
        line.pop_back();
    }
}
