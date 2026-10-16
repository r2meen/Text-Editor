#ifndef UNDO_MANAGER_H
#define UNDO_MANAGER_H

#include <string>
#include "Stack.h"

// ------------------------------------------------------------
// Types of editing operations that can be undone
// ------------------------------------------------------------

enum class OpType {
    INSERT_TEXT,   // text inserted inside a line
    DELETE_TEXT,   // text (char / word / range) deleted inside a line
    REPLACE_TEXT,  // text replaced (search & replace)
    INSERT_LINE,   // a whole line inserted
    DELETE_LINE    // a whole line deleted
};

// ------------------------------------------------------------
// One recorded editing operation
//   INSERT_TEXT : text    = inserted text
//   DELETE_TEXT : text    = deleted text
//   REPLACE_TEXT: text    = old text, newText = replacement text
//   INSERT_LINE : text    = content of inserted line
//   DELETE_LINE : text    = content of deleted line
// line / col are 0-based. col is ignored for line operations.
// groupId != 0 means the op belongs to a group (e.g. Replace All)
// and the whole group is undone in one step.
// ------------------------------------------------------------

struct EditOperation {
    OpType type;
    int line;
    int col;
    std::string text;
    std::string newText;
    int groupId;
};

// ------------------------------------------------------------
// Interface the Document class (Member 1) implements so the
// undo system can reverse operations. Member 1's linked-list
// Document just needs to inherit this and provide these 4
// functions (or main.cpp can wrap the Document in an adapter).
// All return false on invalid position.
// ------------------------------------------------------------


class UndoableDocument {
public:
    virtual ~UndoableDocument() = default;
    virtual bool insertText(int line, int col, const std::string& text) = 0;
    virtual bool deleteText(int line, int col, int length) = 0;
    virtual bool insertLine(int index, const std::string& text) = 0;
    virtual bool deleteLine(int index) = 0;
};

// ------------------------------------------------------------
// UndoManager - stores history on a Stack and undoes edits
// ------------------------------------------------------------

class UndoManager {
private:
    Stack<EditOperation> history;
    int nextGroupId;
    int activeGroup;   // 0 = no group open
    int groupDepth;    // supports nested begin/endGroup calls
    bool undoing;      // true while undo() is running
    std::string message;

    bool record(OpType type, int line, int col,
                const std::string& text, const std::string& newText);
    bool applyInverse(UndoableDocument& doc, const EditOperation& op);
    static std::string describe(const EditOperation& op);

public:
    UndoManager();

    // ---- recording (call these after a successful edit) ----
    bool recordInsert(int line, int col, const std::string& insertedText);
    bool recordDelete(int line, int col, const std::string& deletedText);
    bool recordReplace(int line, int col, const std::string& oldText,
                       const std::string& newText);
    bool recordLineInsert(int index, const std::string& lineText);
    bool recordLineDelete(int index, const std::string& lineText);

    // ---- grouping (e.g. Replace All, multi-line range delete) ----
    void beginGroup();
    void endGroup();

    // ---- undo ----
    bool undo(UndoableDocument& doc);

    // ---- info ----
    bool canUndo() const;
    int historySize() const;
    std::string peekDescription() const;
    std::string lastMessage() const;
    bool isUndoing() const;
    void clear();   // call on New / Open document
};

#endif 