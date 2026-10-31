#ifndef SEARCH_H
#define SEARCH_H

#include <cstddef>
#include <string>
#include <vector>

class Document;

struct SearchLocation {
    std::size_t line;
    std::size_t column;
};

class Search {
public:
    // Search a normal string
    static std::vector<SearchLocation> findAll(
        const std::string& document,
        const std::string& searchText
    );

    // Search the actual linked-list document
    static std::vector<SearchLocation> findAll(
        const Document& document,
        const std::string& searchText
    );

    static bool contains(
        const std::string& document,
        const std::string& searchText
    );

    static bool contains(
        const Document& document,
        const std::string& searchText
    );

    static SearchLocation findFirst(
        const std::string& document,
        const std::string& searchText
    );

    static SearchLocation findFirst(
        const Document& document,
        const std::string& searchText
    );
};

#endif
