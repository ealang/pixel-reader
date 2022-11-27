#include "../epub_metadata.h"

#include <gtest/gtest.h>

TEST(EPUB_METADATA, epub_get_ncx__invalid_xml)
{
    std::vector<NavPoint> navmap;
    ASSERT_FALSE(epub_get_ncx("root/toc.ncx", "", navmap));
}

TEST(EPUB_METADATA, epub_get_ncx__navmap_not_found)
{
    const char *xml = (
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ncx>"
        "</ncx>"
    );
    std::vector<NavPoint> navmap;
    ASSERT_FALSE(epub_get_ncx("root/toc.ncx", xml, navmap));
}

TEST(EPUB_METADATA, epub_get_ncx__parses_correct)
{
    const char *xml = (
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ncx>"
          "<head></head>"
          "<docTitle></docTitle>"
          "<navMap>"
            "<navPoint>"
              "<navLabel>"
                "<text>Top 1</text>"
              "</navLabel>"
              "<content src=\"subdir/top1.html\"/>"
              "<navPoint>"
                "<navLabel>"
                  "<text>  Sub 2  </text>"
                "</navLabel>"
                "<content src=\"subdir/sub2.html#fragment\"/>"
              "</navPoint>"
            "</navPoint>"
            "<navPoint>"
              "<navLabel>"
                "<text>Top 3</text>"
              "</navLabel>"
              "<content src=\"sub3.html\"/>"
            "</navPoint>"
          "</navMap>"
        "</ncx>"
    );

    std::vector<NavPoint> expected_navmap = {
        {
            "Top 1",
            "subdir/top1.html",
            "root/subdir/top1.html",
            {
                {
                    "Sub 2",
                    "subdir/sub2.html#fragment",
                    "root/subdir/sub2.html#fragment"
                }
            }
        },
        {
            "Top 3",
            "sub3.html",
            "root/sub3.html"
        }
    };

    
    std::vector<NavPoint> navmap;
    ASSERT_TRUE(epub_get_ncx("root/toc.ncx", xml, navmap));
    ASSERT_EQ(navmap, expected_navmap);
}

TEST(EPUB_METADATA, epub_get_ncx__ignores_incomplete_navpoints)
{
    const char *xml = (
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ncx>"
          "<navMap>"
            "<navPoint>"
              "<navLabel>"
                "<text>Correct</text>"
              "</navLabel>"
              "<content src=\"correct.html\"/>"
            "</navPoint>"
            "<navPoint>"
              "<navLabel>"
              "</navLabel>"
              "<content src=\"missing_label.html\"/>"
            "</navPoint>"
            "<navPoint>"
              "<content src=\"missing_label.html\"/>"
            "</navPoint>"
            "<navPoint>"
              "<navLabel>"
                "<text>Missing content</text>"
              "</navLabel>"
            "</navPoint>"
            "<navPoint>"
              "<navLabel>"
                "<text>Missing content attr</text>"
              "</navLabel>"
              "<content />"
            "</navPoint>"
            "<navPoint>"
              "<navLabel>"
                "<text>Content attr empty</text>"
              "</navLabel>"
              "<content src=\"\"/>"
            "</navPoint>"
          "</navMap>"
        "</ncx>"
    );

    std::vector<NavPoint> expected_navmap = {
        {
            "Correct",
            "correct.html",
            "root/correct.html"
        }
    };
    
    std::vector<NavPoint> navmap;
    ASSERT_TRUE(epub_get_ncx("root/toc.ncx", xml, navmap));
    ASSERT_EQ(navmap, expected_navmap);
}
