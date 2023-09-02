#ifndef PROGRESS_REPORTING_H_
#define PROGRESS_REPORTING_H_

#include <optional>
#include <string>

enum class ProgressReporting {
    CHAPTER_PERCENT,
    GLOBAL_PERCENT
};

ProgressReporting get_next_progress_reporting(ProgressReporting progress_reporting);

std::optional<ProgressReporting> decode_progress_reporting(std::string progress_reporting);
std::string encode_progress_reporting(ProgressReporting progress_reporting);

#endif
