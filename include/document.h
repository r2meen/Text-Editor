
#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <string>
using namespace std;

struct Node {
    string text;
    Node* next;

    Node(const string& value);
};

class Document {
private:
    Node* head;
    int lineCount;

    Node* getLineNode(int lineNumber) const;

public:
    Document();
    ~Document();

    void clear();
    bool isEmpty() const;

    int getLineCount() const;
    int getCharacterCount() const;

    void display() const;

    void insertLine(int lineNumber, const string& text);
    void insertText(int lineNumber, int position, const string& text);

    void deleteLine(int lineNumber);
    void deleteCharacter(int lineNumber, int position);
    void deleteWord(int lineNumber, const string& word);
    void deleteRange(int lineNumber, int startPosition, int length);

    void showCounts() const;
};

#endif
