#include "sys/filesystem_path.h"

#include <gtest/gtest.h>

// fs_path_strip_character

std::string fs_path_strip_character(std::string path, char character);

TEST(FS_PATH_STRIP_SLASH, empty)
{
    EXPECT_EQ(fs_path_strip_character("", '/'), "");
}

TEST(FS_PATH_STRIP_SLASH, strips_characters)
{
    EXPECT_EQ(fs_path_strip_character("//", '/'), "");
    EXPECT_EQ(fs_path_strip_character("//foo/bar//", '/'), "foo/bar");
}

// fs_path_split_dir

using string_pair = std::pair<std::string, std::string>;

static void expect_eq_pairs(const string_pair &a, const string_pair &b)
{
    EXPECT_EQ(a.first, b.first) << "first element didn't match";
    EXPECT_EQ(a.second, b.second) << "second element didn't match";
}

TEST(FS_PATH_SPLIT_DIR, split_dir_absolute)
{
    expect_eq_pairs(fs_path_split_dir("/foo/bar/baz/"), std::make_pair("/foo/bar", "baz"));
    expect_eq_pairs(fs_path_split_dir("/foo/bar/baz"), std::make_pair("/foo/bar", "baz"));
    expect_eq_pairs(fs_path_split_dir("/foo"), std::make_pair("/", "foo"));
    expect_eq_pairs(fs_path_split_dir("/"), std::make_pair("/", ""));
}

TEST(FS_PATH_SPLIT_DIR, split_dir_relative)
{
    expect_eq_pairs(fs_path_split_dir("foo/bar/baz"), std::make_pair("foo/bar", "baz"));
    expect_eq_pairs(fs_path_split_dir("foo/bar/baz"), std::make_pair("foo/bar", "baz"));
    expect_eq_pairs(fs_path_split_dir("foo"), std::make_pair("", "foo"));
    expect_eq_pairs(fs_path_split_dir(""), std::make_pair("", ""));
}

// fs_path_join

TEST(FS_PATH_JOIN, join_absolute)
{
    EXPECT_EQ(fs_path_join("/foo", "bar"), "/foo/bar");
}

TEST(FS_PATH_JOIN, join_relative)
{
    EXPECT_EQ(fs_path_join("foo", "bar"), "foo/bar");
}

TEST(FS_PATH_JOIN, join_empty)
{
    EXPECT_EQ(fs_path_join("", "bar"), "bar");
    EXPECT_EQ(fs_path_join("bar", ""), "bar");
}

// fs_path_parent

TEST(FS_PATH_PARENT, parent_absolute)
{
    EXPECT_EQ(fs_path_parent("/foo/bar/baz"), "/foo/bar");
    EXPECT_EQ(fs_path_parent("/foo"), "/");
    EXPECT_EQ(fs_path_parent("/"), "/");
}

TEST(FS_PATH_PARENT, parent_relative)
{
    EXPECT_EQ(fs_path_parent("foo/bar/baz"), "foo/bar");
    EXPECT_EQ(fs_path_parent("foo"), "");
    EXPECT_EQ(fs_path_parent(""), "");
}

// fs_path_make_absolute

TEST(FS_PATH_MAKE_ABSOLUTE, already_absolute)
{
    EXPECT_EQ(fs_path_make_absolute("/foo/bar", "/baz"), "/foo/bar");
}

TEST(FS_PATH_MAKE_ABSOLUTE, relative)
{
    EXPECT_EQ(fs_path_make_absolute("foo/bar", "/baz"), "/baz/foo/bar");
    EXPECT_EQ(fs_path_make_absolute("", "/baz"), "/baz");
    EXPECT_EQ(fs_path_make_absolute(".", "/baz"), "/baz");
}
