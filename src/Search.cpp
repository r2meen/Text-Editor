#include "../include/Search.h"
#include "../include/Document.h"
#include <cctype>

static bool isWordCharacter(char ch) {
    return std::isalnum(
        static_cast<unsigned char>(ch)
    ) || ch == '_';
}

static bool matchesIgnoreCase(
    const std::string& text,
    std::size_t position,
    const std::string& searchText
) {
    if (position + searchText.length() > text.length())
        return false;

    for (std::size_t j = 0; j < searchText.length(); j++) {
        unsigned char a =
            static_cast<unsigned char>(text[position + j]);

        unsigned char b =
            static_cast<unsigned char>(searchText[j]);

        if (std::tolower(a) != std::tolower(b))
            return false;
    }

    return true;
}

static bool isWholeWord(
    const std::string& text,
    std::size_t position,
    const std::string& searchText
) {
    bool leftBoundary =
        position == 0 ||
        !isWordCharacter(text[position - 1]);

    std::size_t endPosition =
        position + searchText.length();

    bool rightBoundary =
        endPosition >= text.length() ||
        !isWordCharacter(text[endPosition]);

    return leftBoundary && rightBoundary;
}

// Search a normal string
std::vector<SearchLocation> Search::findAll(
    const std::string& document,
    const std::string& searchText
) {
    std::vector<SearchLocation> locations;

    if (searchText.empty())
        return locations;

    std::size_t line = 1;
    std::size_t column = 1;

    for (std::size_t i = 0; i < document.length(); i++) {

        if (document[i] == '\n') {
            line++;
            column = 1;
            continue;
        }

        if (matchesIgnoreCase(document, i, searchText) &&
            isWholeWord(document, i, searchText)) {

            locations.push_back({line, column});
        }

        column++;
    }

    return locations;
}

// Search the linked-list Document
std::vector<SearchLocation> Search::findAll(
    const Document& document,
    const std::string& searchText
) {
    std::vector<SearchLocation> locations;

    if (searchText.empty())
        return locations;

    for (int i = 0; i < document.getLineCount(); i++) {

        std::string lineText = document.getLine(i);

        for (std::size_t j = 0;
             j + searchText.length() <= lineText.length();
             j++) {

            if (matchesIgnoreCase(lineText, j, searchText) &&
                isWholeWord(lineText, j, searchText)) {

                locations.push_back({
                    static_cast<std::size_t>(i + 1),
                    j + 1
                });
            }
        }
    }

    return locations;
}

bool Search::contains(
    const std::string& document,
    const std::string& searchText
) {
    return !findAll(document, searchText).empty();
}

bool Search::contains(
    const Document& document,
    const std::string& searchText
) {
    return !findAll(document, searchText).empty();
}

SearchLocation Search::findFirst(
    const std::string& document,
    const std::string& searchText
) {
    auto results = findAll(document, searchText);

    if (results.empty())
        return {0, 0};

    return results[0];
}

SearchLocation Search::findFirst(
    const Document& document,
    const std::string& searchText
) {
    auto results = findAll(document, searchText);

    if (results.empty())
        return {0, 0};

    return results[0];
}
