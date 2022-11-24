#ifndef UTF_H_
#define UTF_H_

// Step to next character in utf-8 encoded string
inline const char *utf8_step(const char *s)
{
    while ((*++s & 0xC0) == 0x80);
    return s;
}

#endif
