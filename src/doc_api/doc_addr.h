#ifndef DOC_ADDR_H_
#define DOC_ADDR_H_

#include <string>

using DocAddr = uint64_t;

DocAddr make_address(uint32_t chapter_num, uint32_t text_num);
DocAddr increment_address(DocAddr addr, uint32_t text_increment);

uint32_t get_chapter_number(DocAddr addr);
uint32_t get_text_number(DocAddr addr);

std::string to_string(const DocAddr &address);

#endif
