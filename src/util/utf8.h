#ifndef UTF_H_
#define UTF_H_

#include <cstdint>

// Step to next character in utf-8 encoded string
inline const char *utf8_step(const char *s)
{
    while ((*++s & 0xC0) == 0x80);
    return s;
}

bool is_valid_utf8(const char *s, uint32_t str_len);

#endif
