#include "prj_tg_json_parser_object.h"
#include <limits>
#include <charconv>

PrjTgJsonParserObject::PrjTgJsonParserObject(PrjTgJsonParserObject *parent)
    : m_parent(parent)
    , m_type(PrjTgJsonParserObjectType_Array)
    , m_jsonObject(false)
    , m_null(false)
{
}

PrjTgJsonParserObject::~PrjTgJsonParserObject()
{
    clear();
}

void PrjTgJsonParserObject::clear()
{
    for (PrjTgJsonParserObject *obj : m_listObjects) {
        delete obj;
    }
    m_listObjects.clear();
    m_type = PrjTgJsonParserObjectType_Array;
    m_jsonObject = false;
    m_null = false;
    m_text.clear();
}

PrjTgJsonParserObject *PrjTgJsonParserObject::addChild()
{
    // reserve first, so push_back can't throw after the child is allocated
    if (m_listObjects.size() == m_listObjects.capacity()) {
        m_listObjects.reserve(m_listObjects.empty() ? 4 : m_listObjects.size()*2);
    }
    PrjTgJsonParserObject *child = new PrjTgJsonParserObject(this);
    m_listObjects.push_back(child);
    return child;
}

size_t PrjTgJsonParserObject::getObjectCount() const
{
    return m_listObjects.size();
}

PrjTgJsonParserObject *PrjTgJsonParserObject::getObject(size_t index) const
{
    if (index < m_listObjects.size()) {
        return m_listObjects[index];
    }
    return nullptr;
}

PrjTgJsonParserObject *PrjTgJsonParserObject::getObject(const std::string &name) const
{
    // only json object members have names, array elements don't
    if (!isJsonObject()) {
        return nullptr;
    }
    for (auto it = m_listObjects.rbegin(); it != m_listObjects.rend(); ++it) {
        if ((*it)->m_name == name) {
            return *it;
        }
    }
    return nullptr;
}

PrjTgJsonParserObject *PrjTgJsonParserObject::getParent() const
{
    return m_parent;
}

std::string PrjTgJsonParserObject::getName() const
{
    return m_name;
}

void PrjTgJsonParserObject::setName(std::string name)
{
    m_name = std::move(name);
}

void PrjTgJsonParserObject::setText(std::string text)
{
    m_text = std::move(text);
}

void PrjTgJsonParserObject::setType(PrjTgJsonParserObjectType type)
{
    m_type = type;
}

PrjTgJsonParserObject::PrjTgJsonParserObjectType PrjTgJsonParserObject::getType() const
{
    return m_type;
}

bool PrjTgJsonParserObject::isJsonObject() const
{
    return m_type == PrjTgJsonParserObjectType_Array && m_jsonObject;
}

void PrjTgJsonParserObject::setJsonObject(bool jsonObject)
{
    m_jsonObject = jsonObject;
}

bool PrjTgJsonParserObject::isNull() const
{
    return m_null;
}

void PrjTgJsonParserObject::setNull()
{
    m_type = PrjTgJsonParserObjectType_Value;
    m_text = "0";
    m_null = true;
}

std::string PrjTgJsonParserObject::getText() const
{
    if (m_type == PrjTgJsonParserObjectType_Text) {
        return m_text;
    }
    return "";
}

std::string PrjTgJsonParserObject::getNumberText() const
{
    if (m_type == PrjTgJsonParserObjectType_Value) {
        return m_text;
    }
    return "";
}

bool PrjTgJsonParserObject::getValue(double &value) const
{
    if (m_type != PrjTgJsonParserObjectType_Value) {
        return false;
    }
    const char *end = m_text.data()+m_text.size();
    auto result = std::from_chars(m_text.data(), end, value);
    return result.ec == std::errc() && result.ptr == end;
}

double PrjTgJsonParserObject::getValue() const
{
    double value;
    if (getValue(value)) {
        return value;
    }
    return -std::numeric_limits<double>::max();
}

bool PrjTgJsonParserObject::getInt64(int64_t &value) const
{
    if (m_type != PrjTgJsonParserObjectType_Value) {
        return false;
    }
    const char *end = m_text.data()+m_text.size();
    auto result = std::from_chars(m_text.data(), end, value);
    return result.ec == std::errc() && result.ptr == end;
}

bool PrjTgJsonParserObject::getBoolean() const
{
    return m_type == PrjTgJsonParserObjectType_Boolean && m_text == "true";
}
