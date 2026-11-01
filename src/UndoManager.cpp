
#include "UndoManager.h"

UndoManager::UndoManager()
    : nextGroupId(1), activeGroup(0),
      undoing(false), message("") {}

// RECORDING OPERATIONS

bool UndoManager::record(OpType type, int line, int col,
                         const std::string& text,
                         const std::string& newText) {

    if (undoing || line < 0 || col < 0)
        return false;

    EditOperation operation;

    operation.type = type;
    operation.line = line;
    operation.col = col;
    operation.text = text;
    operation.newText = newText;
    operation.groupId = activeGroup;

    history.push(operation);

    return true;
}

bool UndoManager::recordInsert(int line, int col,
                               const std::string& insertedText) {
    if (insertedText.empty())
        return false;

    return record(OpType::INSERT_TEXT, line, col, insertedText, "");
}

bool UndoManager::recordDelete(int line, int col,
                               const std::string& deletedText) {
    if (deletedText.empty())
        return false;

    return record(OpType::DELETE_TEXT, line, col, deletedText, "");
}

bool UndoManager::recordReplace(int line, int col,
                                const std::string& oldText,
                                const std::string& newText) {
    if (oldText == newText)
        return false;

    return record(OpType::REPLACE_TEXT, line, col, oldText, newText);
}

bool UndoManager::recordLineInsert(int index,
                                   const std::string& lineText) {
    return record(OpType::INSERT_LINE, index, 0, lineText, "");
}

bool UndoManager::recordLineDelete(int index,
                                   const std::string& lineText) {
    return record(OpType::DELETE_LINE, index, 0, lineText, "");
}


// GROUPING OPERATIONS

void UndoManager::beginGroup() {
    if (undoing)
        return;

    if (activeGroup == 0)
        activeGroup = nextGroupId++;
}

void UndoManager::endGroup() {
    activeGroup = 0;
}


// APPLY UNDO

bool UndoManager::applyUndo(UndoableDocument& doc,
                            const EditOperation& operation) {

    switch (operation.type) {

        case OpType::INSERT_TEXT:
            return doc.deleteText(
                operation.line,
                operation.col,
                static_cast<int>(operation.text.length())
            );

        case OpType::DELETE_TEXT:
            return doc.insertText(
                operation.line,
                operation.col,
                operation.text
            );

        case OpType::REPLACE_TEXT:

            if (!doc.deleteText(
                    operation.line,
                    operation.col,
                    static_cast<int>(operation.newText.length())))
                return false;

            return doc.insertText(
                operation.line,
                operation.col,
                operation.text
            );

        case OpType::INSERT_LINE:
            return doc.deleteLine(operation.line);

        case OpType::DELETE_LINE:
            return doc.insertLine(
                operation.line,
                operation.text
            );
    }

    return false;
}


// UNDO OPERATION

bool UndoManager::undo(UndoableDocument& doc) {

    if (history.isEmpty()) {
        message = "Nothing to undo.";
        return false;
    }

    activeGroup = 0;
    undoing = true;

    EditOperation operation = history.top();

    int groupId = operation.groupId;
    int changesUndone = 0;

    std::string description = describe(operation);

    bool success = true;

    while (!history.isEmpty()) {

        operation = history.top();

        if (changesUndone > 0 &&
            (groupId == 0 || operation.groupId != groupId))
            break;

        if (!applyUndo(doc, operation)) {
            success = false;
            break;
        }

        history.pop();
        changesUndone++;
    }

    undoing = false;

    if (!success) {
        message = "Undo failed: document does not match history.";
        return false;
    }

    message = "Undone: " + description;

    if (changesUndone > 1)
        message += " (" + std::to_string(changesUndone) + " changes)";

    return true;
}


// OPERATION DESCRIPTION

std::string UndoManager::describe(const EditOperation& operation) {

    std::string position =
        "line " + std::to_string(operation.line + 1);

    switch (operation.type) {

        case OpType::INSERT_TEXT:
            return "insert \"" + operation.text + "\" at " + position;

        case OpType::DELETE_TEXT:
            return "delete \"" + operation.text + "\" at " + position;

        case OpType::REPLACE_TEXT:
            return "replace \"" + operation.text +
                   "\" with \"" + operation.newText +
                   "\" at " + position;

        case OpType::INSERT_LINE:
            return "insert " + position;

        case OpType::DELETE_LINE:
            return "delete " + position;
    }

    return "unknown operation";
}


// INFORMATION FUNCTIONS

bool UndoManager::canUndo() const {
    return !history.isEmpty();
}

int UndoManager::historySize() const {
    return history.size();
}

std::string UndoManager::peekDescription() const {
    if (history.isEmpty())
        return "Nothing to undo.";

    return describe(history.top());
}

std::string UndoManager::lastMessage() const {
    return message;
}

bool UndoManager::isUndoing() const {
    return undoing;
}


// CLEAR HISTORY

void UndoManager::clear() {

    history.clear();

    activeGroup = 0;
    undoing = false;

    message = "";
}
