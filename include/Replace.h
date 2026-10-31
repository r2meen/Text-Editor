#ifndef REPLACE_H
#define REPLACE_H

#include <cstddef>
#include <string>

class Document;
class UndoManager;

class Replace {
public:
    // Normal string replacement
    static bool replaceFirst(
        std::string& document,
        const std::string& oldText,
        const std::string& newText
    );

    static std::size_t replaceAll(
        std::string& document,
        const std::string& oldText,
        const std::string& newText
    );

    // Replacement directly inside Document
    static bool replaceFirst(
        Document& document,
        const std::string& oldText,
        const std::string& newText,
        UndoManager* undo = nullptr
    );

    static std::size_t replaceAll(
        Document& document,
        const std::string& oldText,
        const std::string& newText,
        UndoManager* undo = nullptr
    );
};

#endif
