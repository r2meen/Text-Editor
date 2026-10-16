#include "UndoManager.h"

UndoManager::UndoManager()
    : nextGroupId(1), activeGroup(0), groupDepth(0), undoing(false), message("") {}

// ------------------------------------------------------------
// Recording
// ------------------------------------------------------------

bool UndoManager::record(OpType type, int line, int col,
                         const std::string& text, const std::string& newText) {
    // Edits made BY undo() must not be recorded again.
    if (undoing) return false;
    if (line < 0 || col < 0) return false;

    EditOperation op;
    op.type = type;
    op.line = line;
    op.col = col;
    op.text = text;
    op.newText = newText;
    op.groupId = activeGroup;
    history.push(op);
    return true;
}

bool UndoManager::recordInsert(int line, int col, const std::string& insertedText) {
    if (insertedText.empty()) return false;
    return record(OpType::INSERT_TEXT, line, col, insertedText, "");
}

bool UndoManager::recordDelete(int line, int col, const std::string& deletedText) {
    if (deletedText.empty()) return false;
    return record(OpType::DELETE_TEXT, line, col, deletedText, "");
}

bool UndoManager::recordReplace(int line, int col, const std::string& oldText,
                                const std::string& newText) {
    if (oldText == newText) return false;
    return record(OpType::REPLACE_TEXT, line, col, oldText, newText);
}

bool UndoManager::recordLineInsert(int index, const std::string& lineText) {
    return record(OpType::INSERT_LINE, index, 0, lineText, "");
}

bool UndoManager::recordLineDelete(int index, const std::string& lineText) {
    return record(OpType::DELETE_LINE, index, 0, lineText, "");
}

// ------------------------------------------------------------
// Grouping
// ------------------------------------------------------------

void UndoManager::beginGroup() {
    if (undoing) return;
    if (groupDepth == 0) {
        activeGroup = nextGroupId++;
    }
    ++groupDepth;
}

void UndoManager::endGroup() {
    if (groupDepth == 0) return;
    --groupDepth;
    if (groupDepth == 0) {
        activeGroup = 0;
    }
}

// ------------------------------------------------------------
// Undo
// ------------------------------------------------------------

bool UndoManager::applyInverse(UndoableDocument& doc, const EditOperation& op) {
    switch (op.type) {
        case OpType::INSERT_TEXT:
            return doc.deleteText(op.line, op.col, static_cast<int>(op.text.length()));

        case OpType::DELETE_TEXT:
            return doc.insertText(op.line, op.col, op.text);

        case OpType::REPLACE_TEXT:
            if (!doc.deleteText(op.line, op.col, static_cast<int>(op.newText.length())))
                return false;
            return doc.insertText(op.line, op.col, op.text);

        case OpType::INSERT_LINE:
            return doc.deleteLine(op.line);

        case OpType::DELETE_LINE:
            return doc.insertLine(op.line, op.text);
    }
    return false;
}

bool UndoManager::undo(UndoableDocument& doc) {
    if (history.isEmpty()) {
        message = "Nothing to undo.";
        return false;
    }

    // An unfinished group is closed before undoing.
    groupDepth = 0;
    activeGroup = 0;

    undoing = true;

    EditOperation op = history.top();
    history.pop();
    std::string desc = describe(op);
    int gid = op.groupId;
    int undone = 0;

    bool ok = applyInverse(doc, op);
    if (ok) ++undone;

    // Undo the rest of the same group (popping in LIFO order keeps
    // positions valid, e.g. Replace All on the same line).
    while (ok && gid != 0 && !history.isEmpty() && history.top().groupId == gid) {
        op = history.top();
        history.pop();
        ok = applyInverse(doc, op);
        if (ok) ++undone;
    }

    undoing = false;

    if (!ok) {
        message = "Undo failed: document no longer matches the history (" + desc + ").";
        return false;
    }

    message = "Undone: " + desc;
    if (undone > 1) {
        message += " (" + std::to_string(undone) + " changes)";
    }
    return true;
}

// ------------------------------------------------------------
// Info
// ------------------------------------------------------------

std::string UndoManager::describe(const EditOperation& op) {
    std::string pos = "line " + std::to_string(op.line + 1);
    switch (op.type) {
        case OpType::INSERT_TEXT:
            return "insert \"" + op.text + "\" at " + pos;
        case OpType::DELETE_TEXT:
            return "delete \"" + op.text + "\" at " + pos;
        case OpType::REPLACE_TEXT:
            return "replace \"" + op.text + "\" -> \"" + op.newText + "\" at " + pos;
        case OpType::INSERT_LINE:
            return "insert " + pos;
        case OpType::DELETE_LINE:
            return "delete " + pos;
    }
    return "unknown operation";
}

bool UndoManager::canUndo() const { return !history.isEmpty(); }

int UndoManager::historySize() const { return history.size(); }

std::string UndoManager::peekDescription() const {
    if (history.isEmpty()) return "Nothing to undo.";
    return describe(history.top());
}

std::string UndoManager::lastMessage() const { return message; }

bool UndoManager::isUndoing() const { return undoing; }

void UndoManager::clear() {
    history.clear();
    groupDepth = 0;
    activeGroup = 0;
    message = "";
}

// testing command
//g++ -std=c++17 -Wall -Wextra -Iinclude tests/undo_test.cpp src/UndoManager.cpp -o undo_test
//./undo_test