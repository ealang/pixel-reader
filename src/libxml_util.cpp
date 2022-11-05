#include "libxml_util.h"

xmlNodePtr elem_first_by_name(xmlNodePtr node, const xmlChar *name)
{
    while(node)
    {
        if (node->type == XML_ELEMENT_NODE && xmlStrEqual(node->name, name))
        {
            break;
        }
        node = node->next;
    }
    return node;
}

xmlNodePtr elem_next_by_name(xmlNodePtr node, const xmlChar *name)
{
    while (node)
    {
        node = node->next;
        if (node && node->type == XML_ELEMENT_NODE && xmlStrEqual(node->name, name))
        {
            break;
        }
    }
    return node;
}

xmlNodePtr elem_first_child(xmlNodePtr node)
{
    if (!node)
    {
        return nullptr;
    }
    return node->children;
}
