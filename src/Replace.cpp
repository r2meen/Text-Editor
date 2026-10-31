#include "../include/Replace.h"
#include "../include/Document.h"
#include "../include/UndoManager.h"

#include <cctype>
#include <vector>

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

// ------------------------------------------------------------
// Replace one match inside the linked-list Document
// ------------------------------------------------------------

static bool replaceAt(
    Document& document,
    int line,
    int col,
    const std::string& oldText,
    const std::string& newText,
    UndoManager* undo
) {
    if (oldText == newText)
        return false;

    if (!document.deleteText(
        line, col, static_cast<int>(oldText.length())
    )) {
        return false;
    }

    if (!newText.empty() &&
        !document.insertText(line, col, newText)) {

        // Restore original text if insertion fails
        document.insertText(line, col, oldText);
        return false;
    }

    // Record only after the edit succeeds
    if (undo != nullptr) {
        if (newText.empty()) {
            undo->recordDelete(line, col, oldText);
        }
        else {
            undo->recordReplace(
                line, col, oldText, newText
            );
        }
    }

    return true;
}

// ------------------------------------------------------------
// Original string-based Replace First
// ------------------------------------------------------------

bool Replace::replaceFirst(
    std::string& document,
    const std::string& oldText,
    const std::string& newText
) {
    if (oldText.empty() || oldText == newText)
        return false;

    for (std::size_t i = 0;
         i + oldText.length() <= document.length();
         i++) {

        if (document[i] == '\n')
            continue;

        if (matchesIgnoreCase(document, i, oldText) &&
            isWholeWord(document, i, oldText)) {

            // Do not allow a match across lines
            if (document.find('\n', i) <
                i + oldText.length()) {
                continue;
            }

            document.replace(i, oldText.length(), newText);
            return true;
        }
    }

    return false;
}

// ------------------------------------------------------------
// Original string-based Replace All
// ------------------------------------------------------------

std::size_t Replace::replaceAll(
    std::string& document,
    const std::string& oldText,
    const std::string& newText
) {
    if (oldText.empty() || oldText == newText)
        return 0;

    std::size_t count = 0;
    std::size_t i = 0;

    while (i + oldText.length() <= document.length()) {

        if (document[i] == '\n') {
            i++;
            continue;
        }

        if (matchesIgnoreCase(document, i, oldText) &&
            isWholeWord(document, i, oldText)) {

            if (document.find('\n', i) <
                i + oldText.length()) {
                i++;
                continue;
            }

            document.replace(i, oldText.length(), newText);

            count++;
            i += newText.length();
        }
        else {
            i++;
        }
    }

    return count;
}

// ------------------------------------------------------------
// Replace First directly inside Document
// ------------------------------------------------------------

bool Replace::replaceFirst(
    Document& document,
    const std::string& oldText,
    const std::string& newText,
    UndoManager* undo
) {
    if (oldText.empty() || oldText == newText)
        return false;

    for (int line = 0;
         line < document.getLineCount();
         line++) {

        std::string lineText = document.getLine(line);

        for (std::size_t col = 0;
             col + oldText.length() <= lineText.length();
             col++) {

            if (matchesIgnoreCase(lineText, col, oldText) &&
                isWholeWord(lineText, col, oldText)) {

                return replaceAt(
                    document,
                    line,
                    static_cast<int>(col),
                    oldText,
                    newText,
                    undo
                );
            }
        }
    }

    return false;
}

// ------------------------------------------------------------
// Replace All directly inside Document
// ------------------------------------------------------------

std::size_t Replace::replaceAll(
    Document& document,
    const std::string& oldText,
    const std::string& newText,
    UndoManager* undo
) {
    if (oldText.empty() || oldText == newText)
        return 0;

    struct Match {
        int line;
        int col;
    };

    std::vector<Match> matches;

    // First collect every matching position
    for (int line = 0;
         line < document.getLineCount();
         line++) {

        std::string lineText = document.getLine(line);

        for (std::size_t col = 0;
             col + oldText.length() <= lineText.length();) {

            if (matchesIgnoreCase(lineText, col, oldText) &&
                isWholeWord(lineText, col, oldText)) {

                matches.push_back({
                    line,
                    static_cast<int>(col)
                });

                // Avoid overlapping matches
                col += oldText.length();
            }
            else {
                col++;
            }
        }
    }

    if (matches.empty())
        return 0;

    // One undo reverses the entire Replace All
    if (undo != nullptr)
        undo->beginGroup();

    std::size_t count = 0;

    // Process from right to left
    for (int i = static_cast<int>(matches.size()) - 1;
         i >= 0;
         i--) {

        if (replaceAt(
            document,
            matches[i].line,
            matches[i].col,
            oldText,
            newText,
            undo
        )) {
            count++;
        }
    }

    if (undo != nullptr)
        undo->endGroup();

    return count;
}
