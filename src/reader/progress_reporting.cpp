#include "./progress_reporting.h"

#include <stdexcept>

#define PROGRESS_STR_CHAPTER_PERCENT "chapter"
#define PROGRESS_STR_GLOBAL_PERCENT "global"

ProgressReporting get_next_progress_reporting(ProgressReporting progress_reporting)
{
    return static_cast<ProgressReporting>(
        (static_cast<int>(progress_reporting) + 1) % 2
    );
}

std::optional<ProgressReporting> decode_progress_reporting(std::string progress_reporting)
{
    if (progress_reporting == PROGRESS_STR_CHAPTER_PERCENT)
    {
        return ProgressReporting::CHAPTER_PERCENT;
    }
    if (progress_reporting == PROGRESS_STR_GLOBAL_PERCENT)
    {
        return ProgressReporting::GLOBAL_PERCENT;
    }
    return {};
}

std::string encode_progress_reporting(ProgressReporting progress_reporting)
{
    switch (progress_reporting)
    {
        case ProgressReporting::CHAPTER_PERCENT:
            return PROGRESS_STR_CHAPTER_PERCENT;
        case ProgressReporting::GLOBAL_PERCENT:
            return PROGRESS_STR_GLOBAL_PERCENT;
        default:
            throw std::runtime_error("Invalid progress value");
    }
}
