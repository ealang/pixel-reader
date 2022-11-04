#ifndef LIBXML_UTIL_H_
#define LIBXML_UTIL_H_

#include <libxml/parser.h>

xmlNodePtr it_first_by_name(xmlNodePtr node, const xmlChar *name);
xmlNodePtr it_first_child(xmlNodePtr node);

# endif
