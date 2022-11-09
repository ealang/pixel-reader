#include "xhtml_parser.h"
#include "libxml_util.h"

#include <libxml/parser.h>

#include <cstring>
#include <functional>
#include <iostream>
#include <set>
#include <unordered_map>

#define DEBUG_LOG(msg) if (context.debug) { std::cerr << std::string(context.node_depth * 2, ' ') << msg << std::endl; }

struct Context
{
    std::vector<std::string> &lines;
    bool debug = false; // show debug messages

    bool fresh_line = true; // no text yet on current line
    int node_depth = 0;     // depth inside any nodes
    int list_depth = 0;     // depth inside ul/ol nodes
    int pre_depth = 0;      // depth inside pre nodes

    Context(std::vector<std::string> &lines)
        : lines(lines)
    {
    }
};

static void _process_h(xmlNodePtr node, Context &context);
static void _process_li(xmlNodePtr node, Context &context);
static void _process_ul(xmlNodePtr node, Context &context);
static void _process_pre(xmlNodePtr node, Context &context);
static void _process_node(xmlNodePtr node, Context &);

static const std::unordered_map<std::string, std::function<void(xmlNodePtr, Context &)>> _element_handlers = {
    {"h1", _process_h},
    {"h2", _process_h},
    {"h3", _process_h},
    {"h4", _process_h},
    {"h5", _process_h},
    {"h6", _process_h},
    {"li", _process_li},
    {"ol", _process_ul},
    {"ul", _process_ul},
    {"pre", _process_pre}
};

static bool _element_is_blocking(const xmlChar *name)
{
    if (!name)
    {
        return false;
    }
    static const std::set<std::string> blocking_elements = {
        "address", "article", "aside", "blockquote", "canvas", "dd", "div", "dl", "dt",
        "fieldset", "figcaption", "figure", "footer", "form", "h1", "h2", "h3", "h4",
        "h5", "h6", "header", "hgroup", "hr", "li", "main", "nav", "noscript", "ol",
        "output", "p", "pre", "section", "table", "tfoot", "ul", "video"
    };
    return blocking_elements.find((const char*)name) != blocking_elements.end();
}

static bool _is_whitespace(char c)
{
    return c == ' ' || c == '\t';
}

static bool _is_newline(char c)
{
    return c == '\n' || c == '\r';
}

// Remove newlines and compact whitespace
static std::string _sanitize_whitespace(std::string str, bool fresh_line)
{
    std::string result;
    bool last_was_space = false;
    bool has_content = false;
    for (std::string::iterator it = str.begin(); it != str.end(); ++it)
    {
        if (!_is_newline(*it)) 
        {
            if (_is_whitespace(*it))
            {
                if (!last_was_space && (has_content || !fresh_line))
                {
                    result.push_back(' ');
                    last_was_space = true;
                }
            }
            else
            {
                result.push_back(*it);
                last_was_space = false;
                has_content = true;
            }
        }
    }

    return result;
}

static void _process_text(xmlNodePtr node, Context &context)
{
    std::string line = context.pre_depth == 0
        ? _sanitize_whitespace((const char*)node->content, context.fresh_line)
        : (const char*)node->content;

    if (!line.empty())
    {
        context.lines.emplace_back(line);
        context.fresh_line = false;

        DEBUG_LOG("\"" << line << "\"");
    }
}

static void _ensure_blocking_padded(xmlNodePtr node, Context &context)
{
    if (_element_is_blocking(node->name) && !context.fresh_line)
    {
        DEBUG_LOG("\\n\\n");
        context.lines.push_back("\n\n");
        context.fresh_line = true;
    }
}

static void _process_h(xmlNodePtr node, Context &context)
{
    int n = node->name ? node->name[1] - '0' : 1;
    context.lines.push_back(std::string(n, '#') + " ");

    _process_node(node->children, context);
}

static void _process_li(xmlNodePtr node, Context &context)
{
    auto indent_level = std::max(0, (context.list_depth - 1) * 2);
    context.lines.push_back(std::string(indent_level, ' ') + "- ");

    _process_node(node->children, context);
}

static void _process_ul(xmlNodePtr node, Context &context)
{
    context.list_depth++;
    _process_node(node->children, context);
    context.list_depth--;
}

static void _process_pre(xmlNodePtr node, Context &context)
{
    context.pre_depth++;
    _process_node(node->children, context);
    context.pre_depth--;
}

static void _process_node(xmlNodePtr node, Context &context)
{
    while (node)
    {
        DEBUG_LOG("<node name=" << node->name << ">");
        context.node_depth++;

        if (node->type == XML_TEXT_NODE)
        {
            _process_text(node, context);
        }
        else if (node->type == XML_ELEMENT_NODE && node->children)
        {
            _ensure_blocking_padded(node, context);

            auto it = _element_handlers.find((const char*)node->name);
            if (it == _element_handlers.end())
            {
                _process_node(node->children, context);
            }
            else
            {
                it->second(node, context);
            }

            _ensure_blocking_padded(node, context);
        }

        context.node_depth--;
        DEBUG_LOG("</node name=" << node->name << ">");
        node = node->next;
    }
}

std::vector<std::string> parse_xhtml_lines(const char *xml_str, std::string name)
{
    xmlDocPtr doc = xmlReadMemory(xml_str, strlen(xml_str), nullptr, nullptr, 0);
    if (doc == nullptr)
    {
        std::cerr << "Unable to parse " << name << " as xml" << std::endl;
        return {};
    }

    xmlNodePtr node = xmlDocGetRootElement(doc);

    node = elem_first_child(elem_first_by_name(node, BAD_CAST "html"));
    node = elem_first_child(elem_first_by_name(node, BAD_CAST "body"));

    std::vector<std::string> lines;
    if (node)
    {
        Context context(lines);
        _process_node(node, context);
    }

    xmlFreeDoc(doc);
    return lines;
}
