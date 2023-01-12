#include "../xhtml_parser.h"

#include <gtest/gtest.h>

static void ASSERT_TOKENS_EQ(const std::vector<std::unique_ptr<DocToken>> &actual_tokens, const std::vector<std::unique_ptr<DocToken>> &expected_tokens)
{
    auto actual_it = actual_tokens.begin();
    auto expected_it = expected_tokens.begin();
    int i = 0;
    while (actual_it != actual_tokens.end() && expected_it != expected_tokens.end())
    {
        const auto &actual = *actual_it;
        const auto &expected = *expected_it;
    
        EXPECT_EQ(actual->type, expected->type) << i << ": Type didn't match";
        EXPECT_EQ(actual->address, expected->address) << i << ": Address didn't match";
        ASSERT_EQ(*actual.get(), *expected.get()) << i << ": Token didn't match";
    
        ++actual_it;
        ++expected_it;
        ++i;
    }

    ASSERT_EQ(actual_tokens.size(), expected_tokens.size());
}

static std::vector<std::unique_ptr<DocToken>> _parse_xhtml_tokens(const char *xml)
{
    std::vector<std::unique_ptr<DocToken>> tokens;
    std::unordered_map<std::string, DocAddr> ids;
    parse_xhtml_tokens(xml, "/base/file.xhtml", 0, tokens, ids);
    return tokens;
}

TEST(XHTML_PARSER, basic_text_valid_xhtml)
{
    const char *xml = (
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
        "<!DOCTYPE html>"
        "<html xmlns=\"http://www.w3.org/1999/xhtml\" xmlns:epub=\"http://www.idpf.org/2007/ops\" xml:lang=\"en\" lang=\"en\">"
        "    <head>\n"
        "        <title>Title</title>\n"
        "    </head>\n"
        "    <body>\n"
        "        Text\n"
        "    </body>\n"
        "</html>"
    );
  
    std::vector<std::unique_ptr<DocToken>> expected_tokens;
    expected_tokens.push_back(std::make_unique<TextDocToken>(0, "Text"));
  
    ASSERT_TOKENS_EQ(
        _parse_xhtml_tokens(xml),
        expected_tokens
    );
}

TEST(XHTML_PARSER, whitespace_compaction)
{
    const char *xml = (
        "<html><body>"
        "  This  <i>  has  </i>  some  <i>extra</i>  white<i> <i/>space  "
        "</body></html>"
    );
  
    std::vector<std::unique_ptr<DocToken>> expected_tokens;
    expected_tokens.push_back(std::make_unique<TextDocToken>(0, "This has some extra white space"));
  
    ASSERT_TOKENS_EQ(
        _parse_xhtml_tokens(xml),
        expected_tokens
    );
}

TEST(XHTML_PARSER, line_break)
{
    const char *xml = (
        "<html><body>"
        "<div>Line 1</div>"
        "<div>Line 2</div>"
        "</body></html>"
    );
  
    std::vector<std::unique_ptr<DocToken>> expected_tokens;
    expected_tokens.push_back(std::make_unique<TextDocToken>(0, "Line 1"));
    expected_tokens.push_back(std::make_unique<TextDocToken>(5, "Line 2"));
  
    ASSERT_TOKENS_EQ(
        _parse_xhtml_tokens(xml),
        expected_tokens
    );
}

TEST(XHTML_PARSER, section_compaction)
{
    const char *xml = (
        "<html><body>"
        "  <p>"
        "    <p>"
        "      <div>"
        "         Some text."
        "      </div>"
        "    </p>"
        "    <p>"
        "       Some more."
        "    </p>"
        "  </p>"
        "</body></html>"
    );
  
    std::vector<std::unique_ptr<DocToken>> expected_tokens;
    expected_tokens.push_back(std::make_unique<TextDocToken>(0,  "Some text."));
    expected_tokens.push_back(std::make_unique<TextDocToken>(9,  ""          ));
    expected_tokens.push_back(std::make_unique<TextDocToken>(9,  "Some more."));
    expected_tokens.push_back(std::make_unique<TextDocToken>(18, ""          ));
  
    ASSERT_TOKENS_EQ(
        _parse_xhtml_tokens(xml),
        expected_tokens
    );
}

TEST(XHTML_PARSER, header_elems)
{
    const char *xml = (
        "<html><body>"
        "  <h1>heading <span>1</span></h1>"
        "  <h1>heading 2</h1>"
        "  <h6>heading 3</h6>"
        "  <p>"
        "    Some text"
        "  </p>"
        "</body></html>"
    );

    std::vector<std::unique_ptr<DocToken>> expected_tokens;
    expected_tokens.push_back(std::make_unique<HeaderDocToken>(0, "heading 1"));
    expected_tokens.push_back(std::make_unique<TextDocToken>(8, ""));
    expected_tokens.push_back(std::make_unique<HeaderDocToken>(8, "heading 2"));
    expected_tokens.push_back(std::make_unique<TextDocToken>(16, ""));
    expected_tokens.push_back(std::make_unique<HeaderDocToken>(16, "heading 3"));
    expected_tokens.push_back(std::make_unique<TextDocToken>(24, ""));
    expected_tokens.push_back(std::make_unique<TextDocToken>(24, "Some text"));
    expected_tokens.push_back(std::make_unique<TextDocToken>(32, ""));

    ASSERT_TOKENS_EQ(
        _parse_xhtml_tokens(xml),
        expected_tokens
    );
}

TEST(XHTML_PARSER, pre_elems)
{
    const char *xml = (
        "<html><body>"
            "<span>start</span>"

            "<pre>line1\r\n"
            "<span>line2</span>\r\n"
            "line3</pre>"

            "<pre>line4</pre>"

            "<span>end</span>"
        "</body></html>"
    );

    std::vector<std::unique_ptr<DocToken>> expected_tokens;
    expected_tokens.push_back(std::make_unique<TextDocToken>(0, "start"              ));
    expected_tokens.push_back(std::make_unique<TextDocToken>(5, ""                   ));
    expected_tokens.push_back(std::make_unique<TextDocToken>(5, "line1\nline2\nline3"));
    expected_tokens.push_back(std::make_unique<TextDocToken>(20, ""                  ));
    expected_tokens.push_back(std::make_unique<TextDocToken>(20, "line4"             ));
    expected_tokens.push_back(std::make_unique<TextDocToken>(25, ""                  ));
    expected_tokens.push_back(std::make_unique<TextDocToken>(25, "end"               ));

    ASSERT_TOKENS_EQ(
        _parse_xhtml_tokens(xml),
        expected_tokens
    );
}

TEST(XHTML_PARSER, image_elems)
{
    const char *xml = (
        "<html><body>"
        "<img src=\"foo.png\"></img>"
        "<img src=\"../bar.png\"></img>"
        "<div>Line 2</div>"
        "</body></html>"
    );

    std::vector<std::unique_ptr<DocToken>> expected_tokens;
    expected_tokens.push_back(std::make_unique<ImageDocToken>(0, "/base/foo.png"));
    expected_tokens.push_back(std::make_unique<ImageDocToken>(1, "/bar.png"));
    expected_tokens.push_back(std::make_unique<TextDocToken>(2, "Line 2"));

    ASSERT_TOKENS_EQ(
        _parse_xhtml_tokens(xml),
        expected_tokens
    );
}

TEST(XHTML_PARSER, capture_ids)
{
    const char *xml = (
        "<html><body>"
        "<p id=\"id1\">text1</p>"
        "<p id=\"id2\">text2</p>"
        "</body></html>"
    );

    std::unordered_map<std::string, DocAddr> expected_ids {
        {"id1", 0},
        {"id2", 5},
    };
  
    std::vector<std::unique_ptr<DocToken>> tokens;
    std::unordered_map<std::string, DocAddr> ids;
    ASSERT_TRUE(parse_xhtml_tokens(xml, "", 0, tokens, ids));

    ASSERT_EQ(expected_ids, ids);
}
