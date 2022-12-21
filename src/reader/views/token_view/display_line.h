#ifndef DISPLAY_LINE_H_
#define DISPLAY_LINE_H_

#include "doc_api/doc_addr.h"
#include <string>

struct DisplayLine
{
    enum class Type
    {
        Text,
        Image,
    };

    DocAddr address;
    uint32_t num_lines;
    Type type;

    DisplayLine(DocAddr address, uint32_t num_lines, Type type);
    virtual ~DisplayLine() = default;
};

struct TextLine: public DisplayLine
{
    std::string text;

    TextLine(DocAddr addr, const std::string& text);
    virtual ~TextLine() = default;
};

struct ImageLine: public DisplayLine
{
    std::string image_path;

    ImageLine(DocAddr addr, uint32_t num_lines, const std::string& image_path);
    virtual ~ImageLine() = default;
};

#endif
