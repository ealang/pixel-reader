#ifndef READER_STYLING_H_
#define READER_STYLING_H_

class ReaderStyling
{
public:
    ReaderStyling();
    virtual ~ReaderStyling();

    TTF_Font *get_loaded_font() const;
    const ColorTheme &get_loaded_theme() const;
};

#endif
