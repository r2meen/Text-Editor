
#include "Document.h"
#include <iostream>

using namespace std;

Node::Node(const string& value) {
    text = value;
    next = nullptr;
}

Document::Document() {
    head = nullptr;
    lineCount = 0;
}

Document::~Document() {
    clear();
}

Node* Document::getLineNode(int lineNumber) const {
    if (lineNumber < 1 || lineNumber > lineCount) {
        return nullptr;
    }

    Node* current = head;

    for (int i = 1; i < lineNumber; i++) {
        current = current->next;
    }

    return current;
}

bool Document::isEmpty() const {
    return head == nullptr;
}

int Document::getLineCount() const {
    return lineCount;
}

int Document::getCharacterCount() const {
    int total = 0;
    Node* current = head;

    while (current != nullptr) {
        total += static_cast<int>(current->text.length());
        current = current->next;
    }

    return total;
}

void Document::display() const {
    if (isEmpty()) {
        cout << "Document is empty.\n";
        return;
    }

    Node* current = head;
    int number = 1;

    while (current != nullptr) {
        cout << number << ": " << current->text << '\n';
        current = current->next;
        number++;
    }
}

void Document::insertLine(int lineNumber, const string& text) {
    if (lineNumber < 1 || lineNumber > lineCount + 1) {
        cout << "Invalid line number.\n";
        return;
    }

    Node* newNode = new Node(text);

    if (lineNumber == 1) {
        newNode->next = head;
        head = newNode;
    } else {
        Node* previous = getLineNode(lineNumber - 1);
        newNode->next = previous->next;
        previous->next = newNode;
    }

    lineCount++;
    cout << "Line inserted successfully.\n";
}

void Document::insertText(
    int lineNumber, int position, const string& text) {

    Node* line = getLineNode(lineNumber);

    if (line == nullptr || position < 0 ||
        position > static_cast<int>(line->text.length())) {
        cout << "Invalid line number or position.\n";
        return;
    }

    line->text.insert(position, text);
    cout << "Text inserted successfully.\n";
}

void Document::deleteLine(int lineNumber) {
    if (lineNumber < 1 || lineNumber > lineCount) {
        cout << "Invalid line number.\n";
        return;
    }

    Node* toDelete;

    if (lineNumber == 1) {
        toDelete = head;
        head = head->next;
    } else {
        Node* previous = getLineNode(lineNumber - 1);
        toDelete = previous->next;
        previous->next = toDelete->next;
    }

    delete toDelete;
    lineCount--;

    cout << "Line deleted successfully.\n";
}

void Document::deleteCharacter(int lineNumber, int position) {
    Node* line = getLineNode(lineNumber);

    if (line == nullptr || position < 0 ||
        position >= static_cast<int>(line->text.length())) {
        cout << "Invalid line number or position.\n";
        return;
    }

    line->text.erase(position, 1);
    cout << "Character deleted successfully.\n";
}

void Document::deleteWord(int lineNumber, const string& word) {
    Node* line = getLineNode(lineNumber);

    if (line == nullptr || word.empty()) {
        cout << "Invalid line number or word.\n";
        return;
    }

    size_t position = line->text.find(word);

    if (position == string::npos) {
        cout << "Text not found.\n";
        return;
    }

    line->text.erase(position, word.length());
    cout << "Text deleted successfully.\n";
}

void Document::deleteRange(
    int lineNumber, int startPosition, int length) {

    Node* line = getLineNode(lineNumber);

    if (line == nullptr || startPosition < 0 ||
        length < 0 ||
        startPosition > static_cast<int>(line->text.length())) {
        cout << "Invalid line number, position, or length.\n";
        return;
    }

    line->text.erase(startPosition, length);
    cout << "Range deleted successfully.\n";
}

void Document::showCounts() const {
    cout << "Total lines: " << getLineCount() << '\n';
    cout << "Total characters: " << getCharacterCount() << '\n';
}

void Document::clear() {
    while (head != nullptr) {
        Node* temp = head;
        head = head->next;
        delete temp;
    }

    lineCount = 0;
}
