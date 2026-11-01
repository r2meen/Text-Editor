
#ifndef UNDO_MANAGER_H
#define UNDO_MANAGER_H

#include <string>
#include "Stack.h"

enum class OpType {
    INSERT_TEXT,
    DELETE_TEXT,
    REPLACE_TEXT,
    INSERT_LINE,
    DELETE_LINE
};

struct EditOperation {
    OpType type;
    int line;
    int col;
    std::string text;
    std::string newText;
    int groupId;
};

class UndoableDocument {
public:
    virtual ~UndoableDocument() = default;

    virtual bool insertText(int line, int col, const std::string& text) = 0;
    virtual bool deleteText(int line, int col, int length) = 0;
    virtual bool insertLine(int index, const std::string& text) = 0;
    virtual bool deleteLine(int index) = 0;
};

class UndoManager {
private:
    Stack<EditOperation> history;

    int nextGroupId;
    int activeGroup;

    bool undoing;
    std::string message;

    bool record(OpType type, int line, int col,
                const std::string& text,
                const std::string& newText);

    bool applyUndo(UndoableDocument& doc,
                   const EditOperation& operation);

    static std::string describe(const EditOperation& operation);

public:
    UndoManager();

    bool recordInsert(int line, int col,
                      const std::string& insertedText);

    bool recordDelete(int line, int col,
                      const std::string& deletedText);

    bool recordReplace(int line, int col,
                       const std::string& oldText,
                       const std::string& newText);

    bool recordLineInsert(int index, const std::string& lineText);
    bool recordLineDelete(int index, const std::string& lineText);

    void beginGroup();
    void endGroup();

    bool undo(UndoableDocument& doc);

    bool canUndo() const;
    int historySize() const;

    std::string peekDescription() const;
    std::string lastMessage() const;

    bool isUndoing() const;
    void clear();
};

#endif
