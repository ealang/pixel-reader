#ifndef LINE_BUFFER_H_
#define LINE_BUFFER_H_

#include "reader/display_lines.h"

#include <optional>
#include <unordered_map>

class LineBuffer
{
    std::unordered_map<int, Line> lines;
    int _line_start = 0;
    int _line_end = 0;
public:

    int line_start() const;
    int line_end() const;
    uint32_t size() const;

    std::optional<DocAddr> last_line_address() const;
    uint32_t get_line_for_address(DocAddr address) const;

    const Line *get_line(int line_num) const;

    void prepend(std::string text, DocAddr addr);
    void append(std::string text, DocAddr addr);
    void clear();
};

#endif
