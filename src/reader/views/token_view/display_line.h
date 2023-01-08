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
        ImageRef,
    };

    DocAddr address;
    Type type;

    DisplayLine(DocAddr address, Type type);
    virtual ~DisplayLine() = default;
};

struct TextLine: public DisplayLine
{
    std::string text;
    bool centered;

    TextLine(DocAddr addr, const std::string& text, bool centered = false);
    virtual ~TextLine() = default;
};

struct ImageLine: public DisplayLine
{
    std::string image_path;
    uint32_t num_lines;
    uint32_t width, height;

    ImageLine(
        DocAddr addr,
        const std::string& image_path,
        uint32_t num_lines,
        uint32_t width,
        uint32_t height
    );
    virtual ~ImageLine() = default;
};


// Reference to an image starting on previous line
struct ImageRefLine: public DisplayLine
{
    uint32_t offset;

    ImageRefLine(DocAddr addr, uint32_t offset);
    virtual ~ImageRefLine() = default;
};
#endif
