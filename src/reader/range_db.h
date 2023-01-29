#ifndef RANGE_DB_H_
#define RANGE_DB_H_

#include "doc_api/doc_addr.h"

#include <list>
#include <tuple>

using Range = std::tuple<DocAddr, DocAddr>;
using Ranges = std::list<Range>;

class RangeDB
{
    Ranges ranges;

public:
    RangeDB(std::list<std::tuple<DocAddr, DocAddr>>);

    bool contains_address(DocAddr addr) const;

    void add_range(DocAddr start, DocAddr end);
    void remove_range(DocAddr start, DocAddr end);

    void sort();
    const Ranges &get_ranges() const;

};

#endif
