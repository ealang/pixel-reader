#include "./range_db.h"

#include <sstream>

#include <map>
#include <stdexcept>

RangeDB::RangeDB(Ranges ranges)
    : ranges(std::move(ranges))
{
}

bool RangeDB::contains_address(DocAddr addr) const
{
    for (const auto &[start, end] : ranges)
    {
        if (addr >= start && addr < end)
        {
            return true;
        }
    }
    return false;
}

void RangeDB::add_range(DocAddr start, DocAddr end)
{
    ranges.push_back(std::make_tuple(start, end));
}

void RangeDB::remove_range(DocAddr del_start, DocAddr del_end)
{
    auto it = ranges.begin();
    while (it != ranges.end())
    {
        auto &[range_start, range_end] = *it;

        if (del_end <= range_start || del_start >= range_end)
        {
            // out of range
            ++it;
        }
        else if (del_start <= range_start && del_end >= range_end)
        {
            // remove entire
            it = ranges.erase(it);
        }
        else if (del_start <= range_start && del_end < range_end)
        {
            // remove start
            range_start = del_end;
            ++it;
        }
        else if (del_start > range_start && del_end >= range_end)
        {
            // remove end
            range_end = del_start;
            ++it;
        }
        else
        {
            // break in two
            ranges.push_front(std::make_tuple(range_start, del_start));
            ranges.push_front(std::make_tuple(del_end, range_end));
            it = ranges.erase(it);
        }

    }
}

void RangeDB::sort()
{
    std::map<DocAddr, int> depth_map;
    for (const auto &[start, end] : ranges)
    {
        depth_map[start] += 1;
        depth_map[end] -= 1;
    }

    std::list<std::tuple<DocAddr, DocAddr>> new_ranges;

    int cur_depth = 0;
    DocAddr cur_start;

    auto it = depth_map.begin();
    while (it != depth_map.end())
    {
        auto &[addr, step] = *it;
        int last_depth = cur_depth;

        cur_depth += step;
        if (cur_depth < 0)
        {
            throw std::runtime_error("Invalid state");
        }

        if (last_depth == 0 && cur_depth > 0)
        {
            cur_start = addr;
        }
        else if (last_depth > 0 && cur_depth == 0)
        {
            new_ranges.push_back(std::make_tuple(cur_start, addr));
        }

        ++it;
    }

    ranges = new_ranges;
}

const Ranges &RangeDB::get_ranges() const
{
    return ranges;
}
