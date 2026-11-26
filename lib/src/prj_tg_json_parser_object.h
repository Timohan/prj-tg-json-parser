/*!
 * \file
 * \brief file prj_tg_json_parser_object.h
 *
 * json parser library's public functionality
 *
 * Copyright of Timo Hannukkala. All rights reserved.
 *
 * \author Timo Hannukkala <timohannukkala@hotmail.com>
 */

#ifndef PRJ_TG_JSON_PARSER_OBJECT_H
#define PRJ_TG_JSON_PARSER_OBJECT_H

#include <cstdint>
#include <string>
#include <vector>

#ifndef TG_JSON_PARSER_LIBRARY_EXPORT
#if defined(__GNUC__) || defined(__clang__)
#define TG_JSON_PARSER_LIBRARY_EXPORT __attribute__((visibility("default")))
#else
#define TG_JSON_PARSER_LIBRARY_EXPORT
#endif
#endif

class PrjTgJsonParserLibPrivate;

/*!
 * \brief PrjTgJsonParserObject
 *
 * One read-only node of the parsed json tree. Nodes are created and owned
 * by PrjTgJsonParserLib, they can't be created, copied, changed or deleted
 * by the user.
 *
 * Json objects and arrays both have type PrjTgJsonParserObjectType_Array,
 * use isJsonObject() to tell them apart.
 * Json null has type PrjTgJsonParserObjectType_Value with value 0, use
 * isNull() to tell it apart from a real 0.
 *
 * Texts and names are UTF-8. A json "\u0000" escape is kept as a NUL byte
 * inside the std::string, so don't use c_str() on texts that may contain it.
 */
class TG_JSON_PARSER_LIBRARY_EXPORT PrjTgJsonParserObject
{
public:
    enum PrjTgJsonParserObjectType {
        PrjTgJsonParserObjectType_Text = 0,
        PrjTgJsonParserObjectType_Value,
        PrjTgJsonParserObjectType_Array,
        PrjTgJsonParserObjectType_Boolean,
    };

    PrjTgJsonParserObject(const PrjTgJsonParserObject &) = delete;
    PrjTgJsonParserObject &operator=(const PrjTgJsonParserObject &) = delete;

    size_t getObjectCount() const;
    //! child by index, nullptr if index is out of range
    PrjTgJsonParserObject *getObject(size_t index) const;
    /*!
     * \brief member of a json object by name, nullptr if not found or this is not a json object
     *
     * If a json object has the same name several times, the last one is
     * returned (same as JavaScript and Python). All of them can be accessed by index.
     */
    PrjTgJsonParserObject *getObject(const std::string &name) const;
    PrjTgJsonParserObject *getParent() const;

    std::string getName() const;
    //! text of a text object, empty string for other types
    std::string getText() const;
    //! number exactly as written in json, e.g. "1.50e3" ("0" for null), empty string for other types
    std::string getNumberText() const;
    //! value of a value object, -std::numeric_limits<double>::max() if not a valid number
    double getValue() const;
    //! value of a value object, returns false if not a value or out of double range
    bool getValue(double &value) const;
    //! exact integer value, returns false if not an integer or out of int64_t range
    bool getInt64(int64_t &value) const;
    bool getBoolean() const;
    PrjTgJsonParserObjectType getType() const;
    //! true if this is a json object ({...}), false for json arrays and other types
    bool isJsonObject() const;
    //! true if this is json null
    bool isNull() const;

private:
    friend class PrjTgJsonParserLibPrivate;

    explicit PrjTgJsonParserObject(PrjTgJsonParserObject *parent);
    ~PrjTgJsonParserObject();

    //! creates a new child owned by this object
    PrjTgJsonParserObject *addChild();
    void setName(std::string name);
    void setText(std::string text);
    void setType(PrjTgJsonParserObjectType type);
    void setJsonObject(bool jsonObject);
    void setNull();
    //! deletes children and resets type and value (name and parent are kept)
    void clear();

    PrjTgJsonParserObject *m_parent;
    PrjTgJsonParserObjectType m_type;
    bool m_jsonObject;
    bool m_null;
    std::string m_name;
    std::string m_text;
    std::vector<PrjTgJsonParserObject *> m_listObjects;
};

#endif // PRJ_TG_JSON_PARSER_OBJECT_H
