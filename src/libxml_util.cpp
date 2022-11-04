#include "libxml_util.h"

xmlNodePtr it_first_by_name(xmlNodePtr node, const xmlChar *name)
{
    if (!node)
    {
        return nullptr;
    }

    for (; node; node=node->next)
    {
        if (xmlStrEqual(node->name, name))
        {
            return node;
        }
    }
    return nullptr;
}

xmlNodePtr it_first_child(xmlNodePtr node)
{
    if (!node)
    {
        return nullptr;
    }
    return node->children;
}
