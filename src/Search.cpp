#include "../include/Search.h"
#include <cctype>

static bool isWordCharacter(char ch)
{
    return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_';
}

static bool isWholeWord(
    const std::string& document,
    std::size_t position,
    const std::string& searchText
)
{
    bool leftBoundary =
        position == 0 ||
        !isWordCharacter(document[position - 1]);

    std::size_t endPosition = position + searchText.length();

    bool rightBoundary =
        endPosition >= document.length() ||
        !isWordCharacter(document[endPosition]);

    return leftBoundary && rightBoundary;
}

std::vector<SearchLocation> Search::findAll(
    const std::string& document,
    const std::string& searchText
)
{
    std::vector<SearchLocation> locations;

    if (searchText.empty())
    {
        return locations;
    }

    std::size_t line = 1;
    std::size_t column = 1;

    for (std::size_t i = 0; i < document.length(); ++i)
    {
        if (document[i] == '\n')
        {
            ++line;
            column = 1;
            continue;
        }

        bool match = true;

        if (i + searchText.length() > document.length())
        {
            match = false;
        }
        else
        {
            for (std::size_t j = 0; j < searchText.length(); ++j)
            {
                unsigned char documentChar =
                    static_cast<unsigned char>(document[i + j]);

                unsigned char searchChar =
                    static_cast<unsigned char>(searchText[j]);

                if (std::tolower(documentChar) !=
                    std::tolower(searchChar))
                {
                    match = false;
                    break;
                }
            }
        }

        if (match && isWholeWord(document, i, searchText))
        {
            locations.push_back({line, column});
        }

        ++column;
    }

    return locations;
}

bool Search::contains(
    const std::string& document,
    const std::string& searchText
)
{
    return !findAll(document, searchText).empty();
}

SearchLocation Search::findFirst(
    const std::string& document,
    const std::string& searchText
)
{
    std::vector<SearchLocation> locations =
        findAll(document, searchText);

    if (locations.empty())
    {
        return {0, 0};
    }

    return locations[0];
}