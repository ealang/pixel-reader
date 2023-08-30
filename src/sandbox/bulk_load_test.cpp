#include "doc_api/token_addressing.h"
#include "filetypes/open_doc.h"
#include "util/timer.h"

#include <filesystem>
#include <iostream>
#include <list>
#include <string>

namespace
{

struct Stats
{
    std::list<std::pair<std::string, uint32_t>> load_times;
};

void load_file(std::filesystem::path path, Stats &stats)
{
    Timer t;

    auto reader = create_doc_reader(path);
    if (!reader || !reader->open())
    {
        std::cerr << "Unable to open " << path.filename() << std::endl;
        return;
    }

    stats.load_times.emplace_back(
        path.filename(), t.elapsed_ms()
    );
}

} // namespace

void bulk_load_test(std::string dir_path)
{
    if (std::filesystem::exists(dir_path) && std::filesystem::is_directory(dir_path))
    {
        Stats stats;

        for (const auto& entry: std::filesystem::directory_iterator(dir_path))
        {
            load_file(entry.path(), stats);
        }

        uint32_t total_load_time = 0;
        for (const auto &[path, load_time]: stats.load_times)
        {
            std::cerr << path << ", " << load_time << std::endl;

            total_load_time += load_time;
        }

        std::cerr << std::endl;
        std::cerr << "Total files: " << stats.load_times.size() << std::endl;
        std::cerr << "Total time: " << total_load_time << std::endl;
    }
    else
    {
        std::cerr << "Invalid directory" << std::endl;
    }
}
