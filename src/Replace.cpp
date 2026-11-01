#include "../include/Replace.h"
#include "../include/Document.h"
#include "../include/Search.h"
#include "../include/UndoManager.h"

// ------------------------------------------------------------
// Replace one match (line / col are 0-based).
// The text that is really in the document is what gets stored
// on the undo stack, so undo restores the original capitalisation
// even when the match was case-insensitive.
// ------------------------------------------------------------
static bool replaceAt(
    Document& document,
    int line,
    int col,
    std::size_t matchLength,
    const std::string& newText,
    UndoManager* undo
) {
    std::string actual =
        document.getText(line, col, static_cast<int>(matchLength));

    if (actual.empty() || actual == newText)
        return false;

    if (!document.deleteText(line, col, static_cast<int>(matchLength)))
        return false;

    if (!newText.empty() && !document.insertText(line, col, newText)) {
        document.insertText(line, col, actual);   // roll back
        return false;
    }

    // Record only after the edit succeeded
    if (undo != nullptr) {
        if (newText.empty())
            undo->recordDelete(line, col, actual);
        else
            undo->recordReplace(line, col, actual, newText);
    }

    return true;
}

bool Replace::replaceNth(
    Document& document,
    const std::string& oldText,
    const std::string& newText,
    std::size_t n,
    UndoManager* undo,
    const MatchOptions& options
) {
    if (oldText.empty() || n == 0)
        return false;

    DynamicArray<SearchLocation> matches =
        Search::findAll(document, oldText, options);

    if (n > matches.size())
        return false;

    const SearchLocation& m = matches[n - 1];

    return replaceAt(document,
                     static_cast<int>(m.line) - 1,
                     static_cast<int>(m.column) - 1,
                     oldText.length(), newText, undo);
}

bool Replace::replaceFirst(
    Document& document,
    const std::string& oldText,
    const std::string& newText,
    UndoManager* undo,
    const MatchOptions& options
) {
    return replaceNth(document, oldText, newText, 1, undo, options);
}

std::size_t Replace::replaceAll(
    Document& document,
    const std::string& oldText,
    const std::string& newText,
    UndoManager* undo,
    const MatchOptions& options
) {
    if (oldText.empty())
        return 0;

    // 1. Collect every match first (O(N * M))
    DynamicArray<SearchLocation> matches =
        Search::findAll(document, oldText, options);

    if (matches.empty())
        return 0;

    // 2. One Undo reverses the entire Replace All
    if (undo != nullptr)
        undo->beginGroup();

    std::size_t count = 0;

    // 3. Right-to-left so earlier columns stay valid
    for (std::size_t i = matches.size(); i > 0; i--) {
        const SearchLocation& m = matches[i - 1];

        if (replaceAt(document,
                      static_cast<int>(m.line) - 1,
                      static_cast<int>(m.column) - 1,
                      oldText.length(), newText, undo)) {
            count++;
        }
    }

    if (undo != nullptr)
        undo->endGroup();

    return count;
}
