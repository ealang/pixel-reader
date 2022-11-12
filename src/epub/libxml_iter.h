#ifndef LIBXML_ITER_H_
#define LIBXML_ITER_H_

#include <libxml/parser.h>

xmlNodePtr elem_first_by_name(xmlNodePtr node, const xmlChar *name);
xmlNodePtr elem_next_by_name(xmlNodePtr node, const xmlChar *name);
xmlNodePtr elem_first_child(xmlNodePtr node);

# endif
