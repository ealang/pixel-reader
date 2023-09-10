#ifndef EPUB_DOC_ADDR_H_
#define EPUB_DOC_ADDR_H_

#include "doc_api/doc_addr.h"

// Epub specific usage of addresses

DocAddr make_address(uint32_t chapter_num = 0, uint32_t text_num = 0);
DocAddr increment_address(DocAddr addr, uint32_t text_increment);

uint32_t get_chapter_number(DocAddr addr);
uint32_t get_text_number(DocAddr addr);

#endif
