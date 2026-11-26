#include "prj_tg_json_parser_lib.h"
#include "prj_tg_json_parser_object.h"
#include "private/prj_tg_json_parser_lib_private.h"

PrjTgJsonParserLib::PrjTgJsonParserLib()
    : m_private(new PrjTgJsonParserLibPrivate())
{
}

PrjTgJsonParserLib::~PrjTgJsonParserLib()
{
    delete m_private;
}

bool PrjTgJsonParserLib::parse(const std::string &filename)
{
    return m_private->parse(filename);
}

bool PrjTgJsonParserLib::parseData(const std::string &data)
{
    return m_private->parseData(data);
}

PrjTgJsonParserObject *PrjTgJsonParserLib::getObject()
{
    return m_private->getObject();
}

PrjTgJsonParserLib::ParseError PrjTgJsonParserLib::getError() const
{
    return m_private->getError();
}

std::string PrjTgJsonParserLib::getErrorMessage() const
{
    return m_private->getErrorMessage();
}

size_t PrjTgJsonParserLib::getErrorLine() const
{
    return m_private->getErrorLine();
}

size_t PrjTgJsonParserLib::getErrorColumn() const
{
    return m_private->getErrorColumn();
}

size_t PrjTgJsonParserLib::getErrorOffset() const
{
    return m_private->getErrorOffset();
}

size_t PrjTgJsonParserLib::getDefaultMaxDepth()
{
    return PrjTgJsonParserLibPrivate::DefaultMaxDepth;
}

void PrjTgJsonParserLib::setMaxDepth(size_t maxDepth)
{
    m_private->setMaxDepth(maxDepth);
}

size_t PrjTgJsonParserLib::getMaxDepth() const
{
    return m_private->getMaxDepth();
}

void PrjTgJsonParserLib::setMaxInputSize(size_t maxInputSize)
{
    m_private->setMaxInputSize(maxInputSize);
}

size_t PrjTgJsonParserLib::getMaxInputSize() const
{
    return m_private->getMaxInputSize();
}
