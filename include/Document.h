
#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <string>
#include "UndoManager.h"

class Document : public UndoableDocument {

private:

    struct LineNode {
        std::string text;
        LineNode* prev;
        LineNode* next;

        LineNode(const std::string& t)
            : text(t), prev(nullptr), next(nullptr) {}
    };

    LineNode* head;
    LineNode* tail;
    int lineCount;

    LineNode* getNode(int index) const;

public:

    Document();
    ~Document();

    Document(const Document&) = delete;
    Document& operator=(const Document&) = delete;

    int getLineCount() const;
    std::string getLine(int index) const;
    std::string getText(int line, int col, int length) const;

    bool findWord(int line, int wordNumber,
                  int& col, int& length) const;

    void display() const;
    int countCharacters() const;

    bool setLine(int index, const std::string& text);

    bool insertText(int line, int col,
                    const std::string& text) override;

    bool deleteText(int line, int col,
                    int length) override;

    bool insertLine(int index,
                    const std::string& text) override;

    bool deleteLine(int index) override;

    bool appendLine(const std::string& text);

    void clear();
};

#endif
