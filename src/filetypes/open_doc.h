#ifndef OPEN_DOC_H_
#define OPEN_DOC_H_

#include "doc_api/doc_reader.h"

#include <filesystem>
#include <memory>

bool file_type_is_supported(const std::filesystem::path &path);
std::shared_ptr<DocReader> create_doc_reader(const std::filesystem::path &path);

#endif
