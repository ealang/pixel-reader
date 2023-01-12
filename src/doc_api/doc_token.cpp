#include "./doc_token.h"

DocToken::DocToken(TokenType type, DocAddr address)
    : type(type), address(address)
{
}

bool DocToken::operator==(const DocToken &other) const
{
    return type == other.type && address == other.address;
}

std::string DocToken::common_to_string(std::string data) const
{
    return (
        "[DocToken "
        "address=" + ::to_string(address) + ", "
        "type=" + ::to_string(type) +
        (
            data.empty()
            ? ""
            : ", data=\"" + data + "\""
        ) +
        "]"
    );
}

////////////////////////

HeaderDocToken::HeaderDocToken(DocAddr address, const std::string &text)
    : DocToken(TokenType::Header, address), text(text)
{
}

bool HeaderDocToken::operator==(const DocToken &other) const
{
    if (!DocToken::operator==(other))
    {
        return false;
    }
    const HeaderDocToken &other_header = static_cast<const HeaderDocToken &>(other);
    return text == other_header.text;
}

std::string HeaderDocToken::to_string() const
{
    return common_to_string(text);
}

////////////////////////

TextDocToken::TextDocToken(DocAddr address, const std::string &text)
    : DocToken(TokenType::Text, address), text(text)
{
}

bool TextDocToken::operator==(const DocToken &other) const
{
    if (!DocToken::operator==(other))
    {
        return false;
    }
    const TextDocToken &other_text = static_cast<const TextDocToken &>(other);
    return text == other_text.text;
}

std::string TextDocToken::to_string() const
{
    return common_to_string(text);
}

////////////////////////

ImageDocToken::ImageDocToken(DocAddr address, const std::filesystem::path &path)
    : DocToken(TokenType::Image, address), path(path)
{
}

bool ImageDocToken::operator==(const DocToken &other) const
{
    if (!DocToken::operator==(other))
    {
        return false;
    }
    const ImageDocToken &other_image = static_cast<const ImageDocToken &>(other);
    return path == other_image.path;
}

std::string ImageDocToken::to_string() const
{
    return common_to_string(path);
}

////////////////////////

ListItemDocToken::ListItemDocToken(DocAddr address, const std::string &text, int nest_level)
    : DocToken(TokenType::ListItem, address), text(text), nest_level(nest_level)
{
}

bool ListItemDocToken::operator==(const DocToken &other) const
{
    if (!DocToken::operator==(other))
    {
        return false;
    }
    const ListItemDocToken &other_list_item = static_cast<const ListItemDocToken &>(other);
    return text == other_list_item.text && nest_level == other_list_item.nest_level;
}

std::string ListItemDocToken::to_string() const
{
    return common_to_string(text);
}

////////////////////////

std::string to_string(TokenType type)
{
    switch (type)
    {
        case TokenType::Text:
            return "Text";
        case TokenType::Header:
            return "Header";
        case TokenType::Image:
            return "Image";
        case TokenType::ListItem:
            return "ListItem";
        default:
            return "Unknown";
    }
}
