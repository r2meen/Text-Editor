
#include "Document.h"
#include <iostream>

// Constructor: initialize an empty document
Document::Document() {
    head = nullptr;
    tail = nullptr;
    lineCount = 0;
}

// Delete all nodes from the document
void Document::clear() {
    LineNode* current = head;

    while (current != nullptr) {
        LineNode* nextNode = current->next;
        delete current;
        current = nextNode;
    }

    head = nullptr;
    tail = nullptr;
    lineCount = 0;
}

// Destructor: free all allocated memory
Document::~Document() {
    clear();
}

// Return the node at the given index
Document::LineNode* Document::getNode(int index) const {
    if (index < 0 || index >= lineCount) {
        return nullptr;
    }

    LineNode* current = head;

    for (int i = 0; i < index; i++) {
        current = current->next;
    }

    return current;
}

// Return the total number of lines
int Document::getLineCount() const {
    return lineCount;
}

// Return the text of a particular line
std::string Document::getLine(int index) const {
    LineNode* node = getNode(index);

    if (node == nullptr) {
        return "";
    }

    return node->text;
}

// Add a new line at the end
bool Document::appendLine(const std::string& text) {
    LineNode* node = new LineNode(text);

    if (head == nullptr) {
        head = node;
        tail = node;
    }
    else {
        node->prev = tail;
        tail->next = node;
        tail = node;
    }

    lineCount++;
    return true;
}

// Insert a line at a particular index
bool Document::insertLine(int index, const std::string& text) {
    if (index < 0 || index > lineCount) {
        return false;
    }

    if (index == lineCount) {
        return appendLine(text);
    }

    LineNode* after = getNode(index);
    LineNode* node = new LineNode(text);

    node->next = after;
    node->prev = after->prev;

    if (after->prev != nullptr) {
        after->prev->next = node;
    }
    else {
        head = node;
    }

    after->prev = node;

    lineCount++;
    return true;
}

// Delete a line at a particular index
bool Document::deleteLine(int index) {
    LineNode* node = getNode(index);

    if (node == nullptr) {
        return false;
    }

    if (node->prev != nullptr) {
        node->prev->next = node->next;
    }
    else {
        head = node->next;
    }

    if (node->next != nullptr) {
        node->next->prev = node->prev;
    }
    else {
        tail = node->prev;
    }

    delete node;
    lineCount--;

    return true;
}

// Replace the complete text of a line
bool Document::setLine(int index, const std::string& text) {
    LineNode* node = getNode(index);

    if (node == nullptr) {
        return false;
    }

    node->text = text;
    return true;
}

// Insert text at a particular column
bool Document::insertText(int line, int col, const std::string& text) {
    LineNode* node = getNode(line);

    if (node == nullptr || col < 0 ||
        col > static_cast<int>(node->text.length())) {
        return false;
    }

    node->text.insert(col, text);
    return true;
}

// Delete a section of text from a line
bool Document::deleteText(int line, int col, int length) {
    LineNode* node = getNode(line);

    if (node == nullptr || col < 0 || length <= 0 ||
        col > static_cast<int>(node->text.length()) ||
        length > static_cast<int>(node->text.length()) - col) {
        return false;
    }

    node->text.erase(col, length);
    return true;
}

// Return a selected portion of text
std::string Document::getText(int line, int col, int length) const {
    LineNode* node = getNode(line);

    if (node == nullptr || col < 0 || length <= 0 ||
        col > static_cast<int>(node->text.length()) ||
        length > static_cast<int>(node->text.length()) - col) {
        return "";
    }

    return node->text.substr(col, length);
}

// Find a word using its zero-based word number
bool Document::findWord(int line, int wordNumber,
                        int& col, int& length) const {
    LineNode* node = getNode(line);

    if (node == nullptr || wordNumber < 0) {
        return false;
    }

    const std::string& text = node->text;
    int currentWord = 0;
    int i = 0;

    while (i < static_cast<int>(text.length())) {

        // Skip spaces
        while (i < static_cast<int>(text.length()) && text[i] == ' ') {
            i++;
        }

        if (i >= static_cast<int>(text.length())) {
            break;
        }

        int start = i;

        // Find the end of the word
        while (i < static_cast<int>(text.length()) && text[i] != ' ') {
            i++;
        }

        if (currentWord == wordNumber) {
            col = start;
            length = i - start;
            return true;
        }

        currentWord++;
    }

    return false;
}

// Display all lines with numbering starting from 1
void Document::display() const {
    LineNode* current = head;
    int lineNumber = 1;

    while (current != nullptr) {
        std::cout << lineNumber << "| " << current->text << std::endl;

        current = current->next;
        lineNumber++;
    }
}

// Count all characters, excluding newline characters
int Document::countCharacters() const {
    int total = 0;
    LineNode* current = head;

    while (current != nullptr) {
        total += static_cast<int>(current->text.length());
        current = current->next;
    }

    return total;
}
