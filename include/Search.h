#ifndef SEARCH_H
#define SEARCH_H

#include <cstddef>
#include <string>
#include <vector>

struct SearchLocation {
    std::size_t line;
    std::size_t column;
};

class Search {
public:
    // Finds every occurrence and returns its line and column.
    static std::vector<SearchLocation> findAll(
        const std::string& document,
        const std::string& searchText
    );

    // Checks whether the search text exists.
    static bool contains(
        const std::string& document,
        const std::string& searchText
    );

    // Finds the first occurrence.
    // Returns line 0, column 0 if nothing is found.
    static SearchLocation findFirst(
        const std::string& document,
        const std::string& searchText
    );
};

#endif