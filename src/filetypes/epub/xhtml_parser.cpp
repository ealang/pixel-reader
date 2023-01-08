#include "./xhtml_parser.h"

#include "./libxml_iter.h"
#include "./xhtml_string_util.h"
#include "./util/str_utils.h"

#include "doc_api/token_addressing.h"

#include <libxml/parser.h>

#include <cstring>
#include <filesystem>
#include <iostream>
#include <set>
#include <unordered_map>

#define DEBUG 0
#define DEBUG_LOG(msg) if (DEBUG) { std::cerr << std::string(node_depth * 2, ' ') << msg << std::endl; }

namespace {

bool element_is_blocking(const xmlChar *name)
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

enum class ElementType
{
    H,
    P,
    Ol,
    Ul,
    Pre,
    Unknown,
    Image,
};

const std::unordered_map<std::string, ElementType> _elem_name_to_enum {
    {"h1",    ElementType::H},
    {"h2",    ElementType::H},
    {"h3",    ElementType::H},
    {"h4",    ElementType::H},
    {"h5",    ElementType::H},
    {"h6",    ElementType::H},
    {"p",     ElementType::P},
    {"ol",    ElementType::Ol},
    {"ul",    ElementType::Ul},
    {"pre",   ElementType::Pre},
    {"img",   ElementType::Image},
    {"image", ElementType::Image},
};

ElementType elem_name_to_enum(const xmlChar *elem_name)
{
    if (!elem_name)
    {
        return ElementType::Unknown;
    }
    auto it = _elem_name_to_enum.find((const char*)elem_name);
    if (it != _elem_name_to_enum.end())
    {
        return it->second;
    }
    return ElementType::Unknown;
}

std::string escape_newlines(const xmlChar *str)
{
    std::string result;
    result.reserve(xmlStrlen(str));

    xmlChar c;
    while ((c = *str++))
    {
        if (c == '\n')
        {
            result += "\\n";
        }
        else
        {
            result += c;
        }
    }
    return result;
}

// Receive callbacks for xhtml nodes, emit DocTokens.
class NodeProcessor
{
    // Intermediate storage of xhtml nodes to be converted to DocTokens
    struct Node
    {
        enum class Type
        {
            InlineText,
            InlineHeader,
            InlineBreak,
            SectionSeparator,
            Image,
        };

        Type type;
        DocAddr address;
        xmlNodePtr node;
        bool is_pre;

        Node(Type type, DocAddr address, xmlNodePtr node, bool is_pre)
            : type(type), address(address), node(node), is_pre(is_pre)
        {
        }

        std::string to_string() const
        {
            std::string typestr;
            switch (type)
            {
                case Type::InlineText:
                    typestr = "InlineText";
                    break;
                case Type::InlineHeader:
                    typestr = "InlineHeader";
                    break;
                case Type::InlineBreak:
                    typestr = "InlineBreak";
                    break;
                case Type::SectionSeparator:
                    typestr = "SectionSeparator";
                    break;
                case Type::Image:
                    typestr = "Image";
                    break;
                default:
                    typestr = "Unknown";
                    break;
            }
            return typestr + " " + ::to_string(address);
        }
    };

    int list_depth = 0;     // depth inside ul/ol tags
    int pre_depth = 0;      // depth inside pre tags
    int header_depth = 0;   // depth inside header tags

    DocAddr current_address;
    std::filesystem::path base_path;  // directory of file (for resolving relative paths)

    std::vector<Node> nodes;
    std::set<std::string> unattached_ids;
    std::unordered_map<std::string, DocAddr> &id_to_addr;

    void attach_pending_ids(DocAddr address)
    {
        for (const auto &id : unattached_ids)
        {
            id_to_addr[id] = address;
        }
        unattached_ids.clear();
    }

    void emit_node(int node_depth, Node::Type type, xmlNodePtr node)
    {
        attach_pending_ids(current_address);
        nodes.emplace_back(type, current_address, node, pre_depth > 0);
        DEBUG_LOG("[node: " << nodes.back().to_string() << "]");
    }

public:
    NodeProcessor(
        DocAddr current_address,
        std::filesystem::path base_path,
        std::unordered_map<std::string, DocAddr> &id_to_addr
    ) : current_address(current_address), base_path(base_path), id_to_addr(id_to_addr)
    {
    }

    void on_text_node(xmlNodePtr node, int node_depth)
    {
        DEBUG_LOG("\"" << escape_newlines(node->content) << "\"");

        if (node->content && xmlStrlen(node->content))
        {
            emit_node(
                node_depth,
                header_depth > 0 ? Node::Type::InlineHeader : Node::Type::InlineText,
                node
            );

            current_address += get_address_width((const char*)node->content);
        }
    }

    void on_enter_element_node(xmlNodePtr node, int node_depth)
    {
        DEBUG_LOG("<node name=\"" << node->name << "\">");

        // Look for id
        {
            const xmlChar *elem_id = xmlGetProp(node, BAD_CAST "id");
            if (elem_id && xmlStrlen(elem_id) > 0)
            {
                unattached_ids.insert((const char*)elem_id);
            }
        }

        if (element_is_blocking(node->name))
        {
            emit_node(node_depth, Node::Type::InlineBreak, node);
        }

        switch (elem_name_to_enum(node->name))
        {
            case ElementType::H:
                emit_node(node_depth, Node::Type::SectionSeparator, node);
                ++header_depth;
                break;
            case ElementType::Ol:
            case ElementType::Ul:
                emit_node(node_depth, Node::Type::SectionSeparator, node);
                ++list_depth;
                break;
            case ElementType::P:
                if (list_depth == 0)
                {
                    emit_node(node_depth, Node::Type::SectionSeparator, node);
                }
                break;
            case ElementType::Pre:
                emit_node(node_depth, Node::Type::SectionSeparator, node);
                ++pre_depth;
                break;
            case ElementType::Image:
                emit_node(node_depth, Node::Type::Image, node);
                break;
            default:
                break;
        }
    }

    void on_exit_element_node(xmlNodePtr node, int node_depth)
    {
        DEBUG_LOG("</node name=\"" << node->name << "\">");

        ElementType elem_type = elem_name_to_enum(node->name);
        switch (elem_type)
        {
            case ElementType::H:
                emit_node(node_depth, Node::Type::SectionSeparator, node);
                --header_depth;
                break;
            case ElementType::Ol:
            case ElementType::Ul:
                emit_node(node_depth, Node::Type::SectionSeparator, node);
                --list_depth;
                break;
            case ElementType::P:
                if (list_depth == 0)
                {
                    emit_node(node_depth, Node::Type::SectionSeparator, node);
                }
                break;
            case ElementType::Pre:
                emit_node(node_depth, Node::Type::SectionSeparator, node);
                --pre_depth;
                break;
            default:
                break;
        }

        if (element_is_blocking(node->name))
        {
            emit_node(node_depth, Node::Type::InlineBreak, node);
        }

        if (elem_type == ElementType::Image)
        {
            ++current_address;
        }
    }

    // Merge inline text types, emit DocTokens
    void generate_doc_tokens(std::vector<DocToken> &tokens_out) const
    {
        auto get_group_size = [&_nodes=this->nodes](uint32_t i) -> uint32_t {
            Node::Type head_type = _nodes[i].type;
            if (head_type == Node::Type::InlineText || head_type == Node::Type::InlineHeader)
            {
                bool head_pre = _nodes[i].is_pre;

                uint32_t size = 1;
                while (
                    ++i < _nodes.size() &&
                    _nodes[i].type == head_type &&
                    _nodes[i].is_pre == head_pre
                )
                {
                    ++size;
                }

                return size;
            }
            return 1;
        };

        uint32_t n = nodes.size();
        tokens_out.reserve(n);

        bool separator_allowed = false;
        uint32_t i = 0;
        while (i < n)
        {
            uint32_t group_size = get_group_size(i);
            const Node &head = nodes[i];
            DocAddr address = head.address;
            Node::Type type = head.type;

            switch (type)
            {
                case Node::Type::InlineText:
                case Node::Type::InlineHeader:
                    // Compact text types
                    if (!head.is_pre)
                    {
                        std::vector<const char*> substrings;
                        substrings.reserve(group_size);
                        for (uint32_t j = i; j < i + group_size; ++j)
                        {
                            const xmlChar *substr = nodes[j].node->content;
                            if (substr)
                            {
                                substrings.push_back((const char*)substr);
                            }
                        }

                        std::string text = compact_strings(substrings);
                        if (text.size())
                        {
                            tokens_out.emplace_back(
                                type == Node::Type::InlineText ? TokenType::Text : TokenType::Header,
                                address,
                                text
                            );
                            separator_allowed = true;
                        }
                    }
                    else
                    {
                        std::vector<std::string> substrings;
                        substrings.reserve(group_size);
                        for (uint32_t j = i; j < i + group_size; ++j)
                        {
                            const xmlChar *substr = nodes[j].node->content;
                            if (substr)
                            {
                                substrings.push_back(remove_carriage_returns((const char*)substr));
                            }
                        }

                        std::string text = join_strings(substrings);
                        if (text.size())
                        {
                            tokens_out.emplace_back(
                                TokenType::Text,
                                nodes[i].address,
                                text
                            );
                            separator_allowed = true;
                        }
                    }
                    break;
                case Node::Type::Image:
                    {
                        xmlNodePtr node = head.node;
                        const xmlChar *img_path = xmlGetProp(node, BAD_CAST "href");
                        if (!img_path) img_path = xmlGetProp(node, BAD_CAST "src");
                        if (img_path)
                        {
                            std::string data = (
                                (base_path / (const char*)img_path).lexically_normal()
                            );
                            tokens_out.emplace_back(TokenType::Image, address, data);
                        }
                        else
                        {
                            std::cerr << "Unable to get link from image" << std::endl;
                        }

                        separator_allowed = true;
                    }
                    break;
                case Node::Type::SectionSeparator:
                    if (separator_allowed)
                    {
                        tokens_out.emplace_back(
                            TokenType::Text,
                            address,
                            ""
                        );

                        separator_allowed = false;
                    }
                    break;
                case Node::Type::InlineBreak:
                    break;
                default:
                    throw std::runtime_error("Unknown node type");
            }
            i += group_size;
        }
    }
};

void visit_nodes(xmlNodePtr node, NodeProcessor &processor, int node_depth = 0)
{
    while (node)
    {
        // Enter handlers
        if (node->type == XML_TEXT_NODE)
        {
            processor.on_text_node(node, node_depth);
        }
        else if (node->type == XML_ELEMENT_NODE)
        {
            processor.on_enter_element_node(node, node_depth);
        }

        // Descend
        visit_nodes(node->children, processor, node_depth + 1);

        // Exit handlers
        if (node->type == XML_ELEMENT_NODE)
        {
            processor.on_exit_element_node(node, node_depth);
        }

        node = node->next;
    }
}

} // namespace

bool parse_xhtml_tokens(const char *xml_str, std::filesystem::path file_path, uint32_t chapter_number, std::vector<DocToken> &tokens_out, std::unordered_map<std::string, DocAddr> &id_to_addr_out)
{
    xmlDocPtr doc = xmlReadMemory(xml_str, strlen(xml_str), nullptr, nullptr, XML_PARSE_NOERROR | XML_PARSE_NOWARNING | XML_PARSE_RECOVER);
    if (doc == nullptr)
    {
        std::cerr << "Unable to parse " << file_path << " as xml" << std::endl;
        return false;
    }

    xmlNodePtr node = xmlDocGetRootElement(doc);

    node = elem_first_child(elem_first_by_name(node, BAD_CAST "html"));
    node = elem_first_child(elem_first_by_name(node, BAD_CAST "body"));

    NodeProcessor processor(
        make_address(chapter_number),
        file_path.parent_path(),
        id_to_addr_out
    );
    visit_nodes(node, processor);

    processor.generate_doc_tokens(tokens_out);
    xmlFreeDoc(doc);

    return true;
}
