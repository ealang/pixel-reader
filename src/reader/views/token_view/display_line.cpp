#include "./display_line.h"

DisplayLine::DisplayLine(DocAddr address, Type type)
    : address(address), type(type)
{
}

TextLine::TextLine(DocAddr addr, const std::string& text, bool centered)
    : DisplayLine{addr, DisplayLine::Type::Text}
    , text{text}
    , centered{centered}
{
}

ImageLine::ImageLine(
    DocAddr addr,
    const std::filesystem::path& image_path,
    uint32_t num_lines,
    uint32_t width,
    uint32_t height
) : DisplayLine{addr, DisplayLine::Type::Image}
  , image_path{image_path}
  , num_lines{num_lines}
  , width{width}
  , height{height}
{
}

ImageRefLine::ImageRefLine(DocAddr addr, uint32_t offset)
    : DisplayLine{addr, DisplayLine::Type::ImageRef}
    , offset{offset}
{
}
