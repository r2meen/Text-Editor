// Tests for Stack and UndoManager 
// Uses a simple mock document so it runs without the other modules.

#include <iostream>
#include <vector>
#include <string>
#include "Stack.h"
#include "UndoManager.h"

static int passed = 0, failed = 0;
#define CHECK(cond, name) \
    do { if (cond) { ++passed; std::cout << "[PASS] " << name << "\n"; } \
         else { ++failed; std::cout << "[FAIL] " << name << "\n"; } } while (0)

// ---- Mock document (stands in for Member 1's linked list) ----
class MockDoc : public UndoableDocument {
public:
    std::vector<std::string> lines;

    bool insertText(int l, int c, const std::string& t) override {
        if (l < 0 || l >= (int)lines.size() || c < 0 || c > (int)lines[l].size()) return false;
        lines[l].insert(c, t);
        return true;
    }
    bool deleteText(int l, int c, int len) override {
        if (l < 0 || l >= (int)lines.size() || c < 0 || len < 0 ||
            c + len > (int)lines[l].size()) return false;
        lines[l].erase(c, len);
        return true;
    }
    bool insertLine(int i, const std::string& t) override {
        if (i < 0 || i > (int)lines.size()) return false;
        lines.insert(lines.begin() + i, t);
        return true;
    }
    bool deleteLine(int i) override {
        if (i < 0 || i >= (int)lines.size()) return false;
        lines.erase(lines.begin() + i);
        return true;
    }
};

void testStack() {
    Stack<int> s;
    CHECK(s.isEmpty() && s.size() == 0, "stack starts empty");
    s.push(1); s.push(2); s.push(3);
    CHECK(s.size() == 3 && s.top() == 3, "push / top");
    s.pop();
    CHECK(s.top() == 2 && s.size() == 2, "pop");
    s.clear();
    CHECK(s.isEmpty(), "clear");
    bool threw = false;
    try { s.pop(); } catch (const std::underflow_error&) { threw = true; }
    CHECK(threw, "pop on empty throws");
}

void testUndoInsert() {
    MockDoc d; d.lines = {"Hello"};
    UndoManager u;
    d.insertText(0, 5, " World"); u.recordInsert(0, 5, " World");
    CHECK(d.lines[0] == "Hello World", "insert applied");
    CHECK(u.undo(d) && d.lines[0] == "Hello", "undo insert");
}

void testUndoDelete() {
    MockDoc d; d.lines = {"Hello World"};
    UndoManager u;
    d.deleteText(0, 5, 6); u.recordDelete(0, 5, " World");
    CHECK(u.undo(d) && d.lines[0] == "Hello World", "undo delete");
}

void testUndoReplace() {
    MockDoc d; d.lines = {"I like cats"};
    UndoManager u;
    d.deleteText(0, 7, 4); d.insertText(0, 7, "dogs");
    u.recordReplace(0, 7, "cats", "dogs");
    CHECK(d.lines[0] == "I like dogs", "replace applied");
    CHECK(u.undo(d) && d.lines[0] == "I like cats", "undo replace");
}

void testUndoReplaceAll() {
    MockDoc d; d.lines = {"a cat and a cat", "cat"};
    UndoManager u;
    u.beginGroup();
    // simulate Replace All "cat" -> "tiger" (left to right)
    d.deleteText(0, 2, 3); d.insertText(0, 2, "tiger"); u.recordReplace(0, 2, "cat", "tiger");
    d.deleteText(0, 14, 3); d.insertText(0, 14, "tiger"); u.recordReplace(0, 14, "cat", "tiger");
    d.deleteText(1, 0, 3); d.insertText(1, 0, "tiger"); u.recordReplace(1, 0, "cat", "tiger");
    u.endGroup();
    CHECK(d.lines[0] == "a tiger and a tiger" && d.lines[1] == "tiger", "replace all applied");
    CHECK(u.undo(d), "undo replace all returns true");
    CHECK(d.lines[0] == "a cat and a cat" && d.lines[1] == "cat", "replace all undone in one step");
    CHECK(!u.canUndo(), "history empty after group undo");
}

void testUndoLines() {
    MockDoc d; d.lines = {"one", "two"};
    UndoManager u;
    d.insertLine(1, "middle"); u.recordLineInsert(1, "middle");
    d.deleteLine(0); u.recordLineDelete(0, "one");
    CHECK(d.lines.size() == 2 && d.lines[0] == "middle", "line ops applied");
    u.undo(d);
    CHECK(d.lines.size() == 3 && d.lines[0] == "one", "undo line delete");
    u.undo(d);
    CHECK(d.lines.size() == 2 && d.lines[1] == "two", "undo line insert");
}

void testEmptyUndo() {
    MockDoc d; d.lines = {"x"};
    UndoManager u;
    CHECK(!u.undo(d), "undo on empty history returns false");
    CHECK(u.lastMessage() == "Nothing to undo.", "empty-stack message");
}

void testOrderAndMisc() {
    MockDoc d; d.lines = {""};
    UndoManager u;
    d.insertText(0, 0, "a"); u.recordInsert(0, 0, "a");
    d.insertText(0, 1, "b"); u.recordInsert(0, 1, "b");
    d.insertText(0, 2, "c"); u.recordInsert(0, 2, "c");
    CHECK(u.historySize() == 3, "history size");
    u.undo(d);
    CHECK(d.lines[0] == "ab", "LIFO order 1");
    u.undo(d);
    CHECK(d.lines[0] == "a", "LIFO order 2");
    CHECK(!u.recordInsert(0, 0, ""), "empty insert not recorded");
    u.clear();
    CHECK(!u.canUndo(), "clear history");
}

void testFailedUndo() {
    MockDoc d; d.lines = {"abc"};
    UndoManager u;
    u.recordInsert(5, 0, "zzz");   // invalid line in document
    CHECK(!u.undo(d), "undo fails gracefully on invalid position");
}

int main() {
    testStack();
    testUndoInsert();
    testUndoDelete();
    testUndoReplace();
    testUndoReplaceAll();
    testUndoLines();
    testEmptyUndo();
    testOrderAndMisc();
    testFailedUndo();
    std::cout << "\n" << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}