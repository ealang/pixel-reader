#include "./xhtml_parser.h"

#include "./libxml_iter.h"
#include "./xhtml_string_util.h"
#include "doc_api/token_addressing.h"

#include <libxml/parser.h>

#include <cstring>
#include <functional>
#include <iostream>
#include <set>
#include <unordered_map>

#define DEBUG 0
#define DEBUG_LOG(msg) if (DEBUG) { std::cerr << std::string(context.node_depth * 2, ' ') << msg << std::endl; }

namespace {

#define EMPTY_STR ""

struct Context
{
    int node_depth = 0;     // depth inside any nodes
    int list_depth = 0;     // depth inside ul/ol nodes
    int pre_depth = 0;      // depth inside pre nodes

    std::function<void(const Context&, TokenType, std::string)> _emit_token;

    void emit_token(TokenType type, std::string text)
    {
        _emit_token(*this, type, text);
    }

    Context(std::function<void(const Context&, TokenType, std::string)> _emit_token)
        : _emit_token(_emit_token)
    {
    }
};

bool _element_is_blocking(const xmlChar *name)
{
    if (!name)
    {
        return false;
    }
    static const std::set<std::string> blocking_elements = {
        "address", "article", "aside", "blockquote", "canvas", "dd", "div", "dl", "dt",
        "fieldset", "figcaption", "figure", "footer", "form", "h1", "h2", "h3", "h4",
        "h5", "h6", "header", "hgroup", "hr", "li", "main", "nav", "noscript", "ol",
        "output", "p", "pre", "section", "table", "tfoot", "ul", "video", "br"
    };
    return blocking_elements.find((const char*)name) != blocking_elements.end();
}

/////////////////////////////

void _on_enter_h(int, xmlNodePtr, Context &context)
{
    context.emit_token(TokenType::Section, EMPTY_STR);
    // TODO: emit header type
}

void _on_exit_h(int, xmlNodePtr, Context &context)
{
    context.emit_token(TokenType::Section, EMPTY_STR);
}

/////////////////////////////

void _on_enter_ul(xmlNodePtr, Context &context)
{
    context.emit_token(TokenType::Section, EMPTY_STR);
}

void _on_exit_ul(xmlNodePtr, Context &context)
{
    context.emit_token(TokenType::Section, EMPTY_STR);
}

/////////////////////////////

void _on_enter_p(xmlNodePtr, Context &context)
{
    if (context.list_depth == 0)
    {
        context.emit_token(TokenType::Section, EMPTY_STR);
    }
}

void _on_exit_p(xmlNodePtr, Context &context)
{
    if (context.list_depth == 0)
    {
        context.emit_token(TokenType::Section, EMPTY_STR);
    }
}

/////////////////////////////

void _on_enter_pre(xmlNodePtr, Context &context)
{
    context.emit_token(TokenType::Section, EMPTY_STR);
    context.pre_depth++;
}

void _on_exit_pre(xmlNodePtr, Context &context)
{
    context.emit_token(TokenType::Section, EMPTY_STR);
    context.pre_depth--;
}

/////////////////////////////

const std::unordered_map<std::string, std::function<void(xmlNodePtr, Context &)>> _on_enter_handlers = {
    {"h1", [](xmlNodePtr node, Context &context) {
        _on_enter_h(1, node, context);
    }},
    {"h2", [](xmlNodePtr node, Context &context) {
        _on_enter_h(2, node, context);
    }},
    {"h3", [](xmlNodePtr node, Context &context) {
        _on_enter_h(3, node, context);
    }},
    {"h4", [](xmlNodePtr node, Context &context) {
        _on_enter_h(4, node, context);
    }},
    {"h5", [](xmlNodePtr node, Context &context) {
        _on_enter_h(5, node, context);
    }},
    {"h6", [](xmlNodePtr node, Context &context) {
        _on_enter_h(6, node, context);
    }},
    {"ol", _on_enter_ul},
    {"ul", _on_enter_ul},
    {"p", _on_enter_p},
    {"pre", _on_enter_pre}
};

const std::unordered_map<std::string, std::function<void(xmlNodePtr, Context &)>> _on_exit_handlers = {
    {"h1", [](xmlNodePtr node, Context &context) {
        _on_exit_h(1, node, context);
    }},
    {"h2", [](xmlNodePtr node, Context &context) {
        _on_exit_h(2, node, context);
    }},
    {"h3", [](xmlNodePtr node, Context &context) {
        _on_exit_h(3, node, context);
    }},
    {"h4", [](xmlNodePtr node, Context &context) {
        _on_exit_h(4, node, context);
    }},
    {"h5", [](xmlNodePtr node, Context &context) {
        _on_exit_h(5, node, context);
    }},
    {"h6", [](xmlNodePtr node, Context &context) {
        _on_exit_h(6, node, context);
    }},
    {"ol", _on_exit_ul},
    {"ul", _on_exit_ul},
    {"p", _on_exit_p},
    {"pre", _on_exit_pre}
};

/////////////////////////////

void on_text_node(xmlNodePtr node, Context &context)
{
    const char *str = (const char*)node->content;
    if (str)
    {
        context.emit_token(
            TokenType::Text,
            context.pre_depth > 0
                ? remove_carriage_returns(str)
                : compact_whitespace(str)
        );
    }
}

void on_enter_element_node(xmlNodePtr node, Context &context)
{
    if (_element_is_blocking(node->name))
    {
        context.emit_token(TokenType::Block, EMPTY_STR);
    }

    auto handler = _on_enter_handlers.find((const char*)node->name);
    if (handler != _on_enter_handlers.end())
    {
        handler->second(node, context);
    }
}

void on_exit_element_node(xmlNodePtr node, Context &context)
{
    if (_element_is_blocking(node->name))
    {
        context.emit_token(TokenType::Block, EMPTY_STR);
    }

    auto handler = _on_exit_handlers.find((const char*)node->name);
    if (handler != _on_exit_handlers.end())
    {
        handler->second(node, context);
    }
}

void _process_node(xmlNodePtr node, Context &context)
{
    while (node)
    {
        DEBUG_LOG("<node name=\"" << node->name << "\">");
        context.node_depth++;

        // Enter handlers
        if (node->type == XML_TEXT_NODE)
        {
            on_text_node(node, context);
        }
        else if (node->type == XML_ELEMENT_NODE)
        {
            on_enter_element_node(node, context);
        }

        // Descend
        _process_node(node->children, context);

        // Exit handlers
        if (node->type == XML_ELEMENT_NODE)
        {
            on_exit_element_node(node, context);
        }

        context.node_depth--;
        DEBUG_LOG("</node name=\"" << node->name << "\">");

        node = node->next;
    }
}


class TokenProcessor
{
    uint32_t chapter_number;
    std::vector<DocToken> &tokens;
    uint32_t address_offset = 0;
    bool fresh_line = true;

public:
    TokenProcessor(uint32_t chapter_number, std::vector<DocToken> &tokens)
        : chapter_number(chapter_number), tokens(tokens)
    {
    }

    void operator()(const Context &context, TokenType type, std::string text)
    {
        if (text.size())
        {
            const std::string *prev_text = (
                !tokens.empty() ? &tokens.back().text : nullptr
            );

            bool strip_left = (
                fresh_line ||
                (
                     is_whitespace(text[0]) &&
                     prev_text->size() &&
                     is_whitespace(prev_text->at(prev_text->size() - 1))
                )
            );

            if (strip_left)
            {
                text = std::string(strip_whitespace_left(text.c_str()));
            }
        }

        DocAddr address = make_address(chapter_number, address_offset);

        if (type == TokenType::Text)
        {
            if (text.size())
            {
                tokens.emplace_back(TokenType::Text, address, text);
                DEBUG_LOG(to_string(tokens.back()));

                fresh_line = false;
            }
        }
        else
        {
            fresh_line = true;

            if (type == TokenType::Block)
            {
                if (!tokens.empty() && tokens.back().type == TokenType::Text)
                {
                    tokens.emplace_back(type, address, EMPTY_STR);
                    DEBUG_LOG(to_string(tokens.back()));
                }
            }
            else if (type == TokenType::Section)
            {
                while (!tokens.empty() && tokens.back().type == TokenType::Block)
                {
                    tokens.pop_back();
                    DEBUG_LOG("pop");
                }
                if (!tokens.empty() && tokens.back().type != TokenType::Section)
                {
                    tokens.emplace_back(type, address, EMPTY_STR);
                    DEBUG_LOG(to_string(tokens.back()));
                }
            }
        }

        // update address
        address_offset += get_address_width(text.c_str());
    }
};

} // namespace

std::vector<DocToken> parse_xhtml_tokens(const char *xml_str, std::string name, uint32_t chapter_number)
{
    xmlDocPtr doc = xmlReadMemory(xml_str, strlen(xml_str), nullptr, nullptr, XML_PARSE_NOERROR | XML_PARSE_NOWARNING | XML_PARSE_RECOVER);
    if (doc == nullptr)
    {
        std::cerr << "Unable to parse " << name << " as xml" << std::endl;
        return {};
    }

    xmlNodePtr node = xmlDocGetRootElement(doc);

    node = elem_first_child(elem_first_by_name(node, BAD_CAST "html"));
    node = elem_first_child(elem_first_by_name(node, BAD_CAST "body"));

    std::vector<DocToken> tokens;
    if (node)
    {
        Context context(TokenProcessor(chapter_number, tokens));
        _process_node(node, context);
    }

    xmlFreeDoc(doc);
    return tokens;
}
