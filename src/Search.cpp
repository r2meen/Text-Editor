#include "../include/Search.h"
#include "../include/Document.h"

DynamicArray<SearchLocation> Search::findAll(
    const Document& document,
    const std::string& searchText,
    const MatchOptions& options
) {
    DynamicArray<SearchLocation> locations;

    if (searchText.empty())
        return locations;

    // Sequential getLine(i) is O(1) amortised thanks to the
    // cursor cache inside Document, so this whole loop is O(N * M).
    for (int i = 0; i < document.getLineCount(); i++) {

        std::string lineText = document.getLine(i);
        std::size_t j = 0;

        while (j + searchText.length() <= lineText.length()) {

            if (TextUtils::isMatch(lineText, j, searchText, options)) {
                SearchLocation loc;
                loc.line = static_cast<std::size_t>(i + 1);
                loc.column = j + 1;
                locations.push_back(loc);

                j += searchText.length();   // no overlapping matches
            }
            else {
                j++;
            }
        }
    }

    return locations;
}

bool Search::contains(
    const Document& document,
    const std::string& searchText,
    const MatchOptions& options
) {
    return !findAll(document, searchText, options).empty();
}

SearchLocation Search::findFirst(
    const Document& document,
    const std::string& searchText,
    const MatchOptions& options
) {
    DynamicArray<SearchLocation> results =
        findAll(document, searchText, options);

    if (results.empty())
        return SearchLocation();

    return results[0];
}
