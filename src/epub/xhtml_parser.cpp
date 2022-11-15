#include "./xhtml_parser.h"
#include "./libxml_iter.h"

#include <libxml/parser.h>

#include <cstring>
#include <functional>
#include <iostream>
#include <set>
#include <unordered_map>

#define DEBUG_LOG(msg) if (context.debug) { std::cerr << std::string(context.node_depth * 2, ' ') << msg << std::endl; }

namespace {

struct Context
{
    std::vector<DocToken> &tokens;
    bool debug = false; // show debug messages

    bool fresh_line = true; // no text yet on current line
    int node_depth = 0;     // depth inside any nodes
    int list_depth = 0;     // depth inside ul/ol nodes
    int pre_depth = 0;      // depth inside pre nodes

    Context(std::vector<DocToken> &tokens)
        : tokens(tokens)
    {
    }
};

class WithToken
{
    Context& context;
    DocToken token;

    void add()
    {
        DEBUG_LOG(token.to_string());
        context.tokens.push_back(token);
    }
public:
    WithToken(Context &context, DocToken token)
        : context(context), token(token)
    {
        add();
    }
    ~WithToken()
    {
        add();
    }
};

void _process_h(xmlNodePtr node, Context &context);
void _process_li(xmlNodePtr node, Context &context);
void _process_ul(xmlNodePtr node, Context &context);
void _process_p(xmlNodePtr node, Context &context);
void _process_pre(xmlNodePtr node, Context &context);
void _process_node(xmlNodePtr node, Context &);

const std::unordered_map<std::string, std::function<void(xmlNodePtr, Context &)>> _element_handlers = {
    {"h1", _process_h},
    {"h2", _process_h},
    {"h3", _process_h},
    {"h4", _process_h},
    {"h5", _process_h},
    {"h6", _process_h},
    {"li", _process_li},
    {"ol", _process_ul},
    {"ul", _process_ul},
    {"p", _process_p},
    {"pre", _process_pre}
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
        "output", "p", "pre", "section", "table", "tfoot", "ul", "video"
    };
    return blocking_elements.find((const char*)name) != blocking_elements.end();
}

inline bool _is_whitespace(char c)
{
    return c == ' ' || c == '\t';
}

inline bool _is_newline(char c)
{
    return c == '\n' || c == '\r';
}

inline bool _is_carriage_return(char c)
{
    return c == '\r';
}

std::string _remove_carriage_returns(const char *source)
{
    std::string result;
    result.reserve(strlen(source));

    char c;
    while ((c = *source))
    {
        if (!_is_carriage_return(c))
        {
            result.push_back(c);
        }
        source++;
    }

    return result;
}

// Remove newlines and compact whitespace
std::string _sanitize_whitespace(std::string str, bool fresh_line)
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

void _process_text(xmlNodePtr node, Context &context)
{
    std::string text = context.pre_depth == 0
        ? _sanitize_whitespace((const char*)node->content, context.fresh_line)
        : _remove_carriage_returns((const char*)node->content);

    if (!text.empty())
    {
        context.tokens.emplace_back(TokenType::Text, text);
        context.fresh_line = false;

        DEBUG_LOG(DocToken::text_token(text).to_string());
    }
}

void _ensure_blocking_padded(xmlNodePtr node, Context &context)
{
    if (_element_is_blocking(node->name) && !context.fresh_line)
    {
        DEBUG_LOG(DocToken::block_token().to_string());
        context.tokens.emplace_back(TokenType::Block);
        context.fresh_line = true;
    }
}

void _process_h(xmlNodePtr node, Context &context)
{
    WithToken section_padded(context, DocToken::section_token());

    int n = node->name ? node->name[1] - '0' : 1;
    context.tokens.emplace_back(TokenType::Text, std::string(n, '#') + " ");

    _process_node(node->children, context);
}

void _process_li(xmlNodePtr node, Context &context)
{
    auto indent_level = std::max(0, (context.list_depth - 1) * 2);
    context.tokens.emplace_back(TokenType::Text, std::string(indent_level, ' ') + "- ");

    _process_node(node->children, context);
}

void _process_ul(xmlNodePtr node, Context &context)
{
    WithToken section_padded(context, DocToken::section_token());

    context.list_depth++;
    _process_node(node->children, context);
    context.list_depth--;
}

void _process_pre(xmlNodePtr node, Context &context)
{
    WithToken section_padded(context, DocToken::section_token());

    context.pre_depth++;
    _process_node(node->children, context);
    context.pre_depth--;
}

void _process_p(xmlNodePtr node, Context &context)
{
    if (context.list_depth == 0)
    {
        WithToken section_padded(context, DocToken::section_token());
        _process_node(node->children, context);
    }
    else
    {
        // if inside a list, don't add section breaks
        _process_node(node->children, context);
    }
}

void _process_node(xmlNodePtr node, Context &context)
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

// Output cleaner tokens. Compact text and break tokens.
// TODO: simplify parsing logic to generate this in a single pass
std::vector<DocToken> cleanup_tokens(const std::vector<DocToken> &tokens)
{
    std::vector<DocToken> out;
    std::vector<std::string> pending_text;

    auto compact_pending_text = [&out, &pending_text]() {
        int len = 0;
        for (auto &s : pending_text)
        {
            len += s.length();
        }
        if (len > 0)
        {
            std::string text;
            text.reserve(len);
            for (auto &s : pending_text)
            {
                text += s;
            }
            out.emplace_back(DocToken::text_token(text));
            pending_text.clear();
        }
    };

    for (const auto &token: tokens)
    {
        if (token.type == TokenType::Text)
        {
            pending_text.push_back(token.text);
        }
        else if (token.type == TokenType::Block)
        {
            compact_pending_text();
            if (!out.empty() && out.back().type == TokenType::Text)
            {
                out.emplace_back(token);
            }
        }
        else
        {
            compact_pending_text();
            while (!out.empty() && out.back().type == TokenType::Block)
            {
                out.pop_back();
            }
            if (!out.empty() && out.back().type != TokenType::Section)
            {
                out.emplace_back(token);
            }
        }
    }

    compact_pending_text();

    return out;
}

} // namespace

std::vector<DocToken> parse_xhtml_tokens(const char *xml_str, std::string name)
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
        Context context(tokens);
        _process_node(node, context);
    }

    xmlFreeDoc(doc);
    return cleanup_tokens(tokens);
}
