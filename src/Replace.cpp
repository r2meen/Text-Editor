#include "../include/Replace.h"
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

static bool matchesIgnoreCase(
    const std::string& document,
    std::size_t position,
    const std::string& searchText
)
{
    for (std::size_t j = 0; j < searchText.length(); ++j)
    {
        unsigned char documentChar =
            static_cast<unsigned char>(document[position + j]);

        unsigned char searchChar =
            static_cast<unsigned char>(searchText[j]);

        if (std::tolower(documentChar) !=
            std::tolower(searchChar))
        {
            return false;
        }
    }

    return true;
}

bool Replace::replaceFirst(
    std::string& document,
    const std::string& oldText,
    const std::string& newText
)
{
    if (oldText.empty())
    {
        return false;
    }

    for (std::size_t i = 0;
         i + oldText.length() <= document.length();
         ++i)
    {
        if (matchesIgnoreCase(document, i, oldText) &&
            isWholeWord(document, i, oldText))
        {
            document.replace(i, oldText.length(), newText);
            return true;
        }
    }

    return false;
}

std::size_t Replace::replaceAll(
    std::string& document,
    const std::string& oldText,
    const std::string& newText
)
{
    if (oldText.empty())
    {
        return 0;
    }

    std::size_t count = 0;
    std::size_t i = 0;

    while (i + oldText.length() <= document.length())
    {
        if (matchesIgnoreCase(document, i, oldText) &&
            isWholeWord(document, i, oldText))
        {
            document.replace(i, oldText.length(), newText);

            ++count;

            i += newText.length();
        }
        else
        {
            ++i;
        }
    }

    return count;
}