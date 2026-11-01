#ifndef REPLACE_H
#define REPLACE_H

#include <cstddef>
#include <string>
#include "TextUtils.h"

class Document;
class UndoManager;

class Replace {
public:
    // Replace the n-th match (n is 1-based). Returns false if there is
    // no n-th match or the text would not change.
    static bool replaceNth(
        Document& document,
        const std::string& oldText,
        const std::string& newText,
        std::size_t n,
        UndoManager* undo = nullptr,
        const MatchOptions& options = MatchOptions()
    );

    static bool replaceFirst(
        Document& document,
        const std::string& oldText,
        const std::string& newText,
        UndoManager* undo = nullptr,
        const MatchOptions& options = MatchOptions()
    );

    // Returns the number of replacements made.
    // One Undo reverses the whole Replace All.
    static std::size_t replaceAll(
        Document& document,
        const std::string& oldText,
        const std::string& newText,
        UndoManager* undo = nullptr,
        const MatchOptions& options = MatchOptions()
    );
};

#endif
