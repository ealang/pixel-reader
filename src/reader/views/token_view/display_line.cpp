#include "./display_line.h"

DisplayLine::DisplayLine(DocAddr address, uint32_t num_lines, Type type)
    : address(address), num_lines(num_lines), type(type)
{
}

TextLine::TextLine(DocAddr addr, const std::string& text)
    : DisplayLine{addr, 1, DisplayLine::Type::Text}
    , text{text}
{
}

ImageLine::ImageLine(DocAddr addr, uint32_t num_lines, const std::string& image_path)
    : DisplayLine{addr, num_lines, DisplayLine::Type::Image}
    , image_path{image_path}
{
}
