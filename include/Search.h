#ifndef SEARCH_H
#define SEARCH_H

#include <cstddef>
#include <string>
#include "DynamicArray.h"
#include "TextUtils.h"

class Document;

// 1-based line and column (0,0 means "not found")
struct SearchLocation {
    std::size_t line = 0;
    std::size_t column = 0;
};

class Search {
public:
    // Every non-overlapping match, in document order.
    // An empty search string returns an empty result.
    // Time: O(N * M) worst case (N = characters in document,
    //       M = pattern length), O(N) typical.
    static DynamicArray<SearchLocation> findAll(
        const Document& document,
        const std::string& searchText,
        const MatchOptions& options = MatchOptions()
    );

    static bool contains(
        const Document& document,
        const std::string& searchText,
        const MatchOptions& options = MatchOptions()
    );

    static SearchLocation findFirst(
        const Document& document,
        const std::string& searchText,
        const MatchOptions& options = MatchOptions()
    );
};

#endif
