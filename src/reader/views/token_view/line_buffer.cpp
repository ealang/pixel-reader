#include "./line_buffer.h"

int LineBuffer::line_start() const
{
    return _line_start;
}

int LineBuffer::line_end() const
{
    return _line_end;
}

uint32_t LineBuffer::size() const
{
    return _line_end - _line_start;
}

std::optional<DocAddr> LineBuffer::last_line_address() const
{
    auto it = lines.find(_line_end - 1);
    if (it == lines.end())
    {
        return std::nullopt;
    }
    return it->second.address;
}

uint32_t LineBuffer::get_line_for_address(DocAddr address) const
{
    int best_line = _line_start;
    for (int line = _line_start; line < _line_end; ++line)
    {
        auto it = lines.find(line);
        if (it != lines.end() && it->second.address <= address)
        {
            best_line = line;
            if (it->second.address == address)
            {
                break;
            }
        }
        else
        {
            break;
        }
    }

    return best_line;
}

const Line *LineBuffer::get_line(int line_num) const
{
    auto it = lines.find(line_num);
    if (it == lines.end())
    {
        return nullptr;
    }
    return &it->second;
}

void LineBuffer::prepend(std::string text, DocAddr addr)
{
    lines.emplace(--_line_start, Line(text, addr));
}

void LineBuffer::append(std::string text, DocAddr addr)
{
    lines.emplace(_line_end++, Line(text, addr));
}

void LineBuffer::clear()
{
    lines.clear();
    _line_start = 0;
    _line_end = 0;
}
