#include <gtest/gtest.h>
#include "sys/filesystem_path.h"

TEST(FilesystemPath, fs_path_join) {
    EXPECT_EQ(fs_path_join("/foo", "bar"), "/foo/bar");
}
