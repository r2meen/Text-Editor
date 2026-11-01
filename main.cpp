#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <filesystem>
#include <system_error>

#include "Document.h"
#include "Search.h"
#include "Replace.h"
#include "UndoManager.h"
#include "FileManager.h"
#include "DynamicArray.h"
#include "TextUtils.h"

using namespace std;
namespace fs = std::filesystem;

// ---------------------------------------------------------------
// Terminal colours
// ---------------------------------------------------------------
namespace Color
{
    const string RESET = "\033[0m";
    const string RED = "\033[31m";
    const string GREEN = "\033[32m";
    const string BRIGHT_GREEN = "\033[92m";
    const string YELLOW = "\033[33m";
    const string BLUE = "\033[34m";
    const string CYAN = "\033[36m";
    const string WHITE = "\033[97m";
    const string BLACK = "\033[30m";
    const string GRAY = "\033[90m";
    const string BOLD = "\033[1m";

    const string BG_WHITE = "\033[47m";
    const string BG_YELLOW = "\033[43m";
}

// ---------------------------------------------------------------
// Layout settings
// ---------------------------------------------------------------
const int SCREEN_WIDTH = 80; // set this to your terminal width
const int MENU_WIDTH = 46;   // width of the menu box and underlines

const vector<string> MENU_LABELS = {
    "New Document",              // 1
    "Open File",                 // 2
    "Save File",                 // 3
    "Save As",                   // 4
    "Display document",          // 5
    "Insert a new line",         // 6
    "Insert text into a line",   // 7
    "Delete text / word(s)",     // 8
    "Delete line(s)",            // 9
    "Search text",               // 10
    "Replace one occurrence",    // 11
    "Replace all occurrences",   // 12
    "Undo last operation",       // 13
    "Exit"                       // 14
};

const int FILE_COUNT = 5;
const int INSERT_COUNT = 2;
const int DELETE_COUNT = 2;
const int SEARCH_COUNT = 1;
const int REPLACE_COUNT = 2;
const int EXIT_CHOICE = 14;

// ---------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------

// Counts characters on screen (█ and ═ are several bytes but one column)
int visibleLength(const string &s)
{
    int n = 0;
    for (unsigned char c : s)
    {
        if ((c & 0xC0) != 0x80)
            n++;
    }
    return n;
}

string spaces(int n)
{
    return string(n > 0 ? n : 0, ' ');
}

string repeat(const string &s, int times)
{
    string result;
    for (int i = 0; i < times; i++)
        result += s;
    return result;
}

// Left margin that centers the menu block
string margin()
{
    return spaces((SCREEN_WIDTH - MENU_WIDTH) / 2);
}

void printCentered(const string &text, const string &color = "")
{
    int left = (SCREEN_WIDTH - visibleLength(text)) / 2;
    cout << spaces(left) << color << text << Color::RESET << '\n';
}

// ---------------------------------------------------------------
// Read an integer safely
// ---------------------------------------------------------------
int readInt(string prompt, bool centered = false)
{
    string input;
    int number;

    while (true)
    {
        cout << (centered ? margin() : "")
             << Color::YELLOW << prompt << Color::RESET;

        if (!getline(cin, input))
        {
            cout << "\nInput stream closed. Exiting Text Editor.\n";
            exit(0);
        }

        stringstream ss(input);

        if (ss >> number && ss.eof())
        {
            return number;
        }

        cout << (centered ? margin() : "")
             << Color::RED
             << "Invalid input! Please enter a number.\n"
             << Color::RESET;
    }
}

// ---------------------------------------------------------------
// Big block-letter banner
// ---------------------------------------------------------------
vector<string> glyph(char c)
{
    switch (c)
    {
    case 'T':
        return {"█████", "  █  ", "  █  ", "  █  ", "  █  "};
    case 'E':
        return {"█████", "█    ", "████ ", "█    ", "█████"};
    case 'X':
        return {"█   █", " █ █ ", "  █  ", " █ █ ", "█   █"};
    case 'D':
        return {"████ ", "█   █", "█   █", "█   █", "████ "};
    case 'I':
        return {"█████", "  █  ", "  █  ", "  █  ", "█████"};
    case 'O':
        return {"█████", "█   █", "█   █", "█   █", "█████"};
    case 'R':
        return {"████ ", "█   █", "████ ", "█  █ ", "█   █"};
    default:
        return {"   ", "   ", "   ", "   ", "   "}; // space
    }
}

void displayBanner(const string &word)
{
    cout << "\n";

    for (int row = 0; row < 5; row++)
    {
        string line;

        for (size_t i = 0; i < word.size(); i++)
        {
            line += glyph(word[i])[row];

            if (i + 1 < word.size())
                line += " ";
        }

        printCentered(line, Color::BRIGHT_GREEN + Color::BOLD);
    }

    cout << "\n";
}

// ---------------------------------------------------------------
// Top bar (menu bar + file tab)
// ---------------------------------------------------------------
void displayTopBar(const string &currentFile, bool modified)
{
    string bar = " File   Edit   Search   Help";

    cout << Color::BG_WHITE << Color::BLACK << bar
         << spaces(SCREEN_WIDTH - visibleLength(bar))
         << Color::RESET << '\n';

    string name = currentFile.empty() ? "Untitled" : currentFile;
    if (modified)
        name += "*";

    cout << Color::BG_YELLOW << Color::BLACK
         << " " << name << " " << Color::RESET << '\n';
}

// Whole header: top bar, banner, tagline
void displayHeader(const string &currentFile, bool modified)
{
    cout << "\n";
    displayTopBar(currentFile, modified);
    displayBanner("TEXT EDITOR");

    printCentered("The Terminal Text Editor", Color::YELLOW);
    cout << "\n";
    printCentered("Easy To Use | Undo History | Search & Replace | File Support",
                  Color::CYAN);
}

// ---------------------------------------------------------------
// Menu pieces
// ---------------------------------------------------------------
void printBox(const string &title)
{
    int inner = MENU_WIDTH - 2;
    int left = (inner - visibleLength(title)) / 2;
    int right = inner - left - visibleLength(title);

    cout << margin() << Color::CYAN
         << "╔" << repeat("═", inner) << "╗\n";

    cout << margin() << "║" << spaces(left)
         << Color::YELLOW << Color::BOLD << title << Color::RESET
         << Color::CYAN << spaces(right) << "║\n";

    cout << margin() << "╚" << repeat("═", inner) << "╝\n"
         << Color::RESET;
}

void printSection(const string &title)
{
    cout << "\n"
         << margin()
         << Color::YELLOW << Color::BOLD << title << Color::RESET << '\n';

    cout << margin() << Color::GRAY
         << repeat("─", MENU_WIDTH) << Color::RESET << '\n';
}

void printItem(int number, const string &label,
               const string &numColor = Color::GREEN)
{
    string num = to_string(number) + ".";

    cout << margin() << "  "
         << Color::CYAN << "•" << Color::RESET << " "
         << numColor << Color::BOLD << num << Color::RESET
         << spaces(4 - (int)num.size())
         << label << '\n';
}

void displayMenu()
{
    cout << "\n";
    printBox("MAIN MENU");

    printSection("File Management");
    for (int i = 0; i < FILE_COUNT; i++)
    {
        printItem(i + 1, MENU_LABELS[i]);
    }

    printSection("Insertion");
    for (int i = FILE_COUNT; i < FILE_COUNT + INSERT_COUNT; i++)
    {
        printItem(i + 1, MENU_LABELS[i]);
    }

    printSection("Deletion");
    for (int i = FILE_COUNT + INSERT_COUNT;
         i < FILE_COUNT + INSERT_COUNT + DELETE_COUNT; i++)
    {
        printItem(i + 1, MENU_LABELS[i]);
    }

    printSection("Search");
    printItem(10, MENU_LABELS[9]);

    printSection("Replace");
    for (int i = 10; i < 10 + REPLACE_COUNT; i++)
    {
        printItem(i + 1, MENU_LABELS[i]);
    }

    printSection("Undo");
    printItem(13, MENU_LABELS[12]);

    printSection("Session");
    printItem(EXIT_CHOICE, MENU_LABELS[EXIT_CHOICE - 1], Color::RED);
    cout << "\n";
}

// ---------------------------------------------------------------
// Bottom status bar
// ---------------------------------------------------------------
void displayStatusBar(Document &doc, UndoManager &undo,
                      const string &currentFile, bool modified)
{
    string name = currentFile.empty() ? "Untitled" : currentFile;
    if (modified)
        name += "*";

    string left = " " + name + " | Lines: " + to_string(doc.getLineCount()) +
                  " | Words: " + to_string(doc.countWords()) +
                  " | Chars: " + to_string(doc.countCharacters()) +
                  " | Undo: " + to_string(undo.historySize());

    string right = modified ? "Modified " : "Saved ";

    int gap = SCREEN_WIDTH - visibleLength(left) - visibleLength(right);

    cout << Color::BG_WHITE << Color::BLACK << left
         << spaces(gap) << right << Color::RESET << '\n';
}

// ---------------------------------------------------------------
// Document display helpers
// ---------------------------------------------------------------
void displayDocument(const Document &doc)
{
    printCentered("------------------- DOCUMENT -------------------",
                  Color::CYAN + Color::BOLD);

    if (doc.getLineCount() == 0)
    {
        printCentered("The document is empty.", Color::YELLOW);
    }
    else
    {
        for (int i = 0; i < doc.getLineCount(); i++)
        {
            printCentered(to_string(i + 1) + ". " + doc.getLine(i));
        }
    }

    printCentered("--------------------------------------------", Color::CYAN);
}

vector<string> captureDocument(const Document &doc)
{
    vector<string> lines;

    for (int i = 0; i < doc.getLineCount(); i++)
    {
        lines.push_back(doc.getLine(i));
    }

    return lines;
}

void displaySnapshot(const vector<string> &lines)
{
    if (lines.empty())
    {
        printCentered("Document is empty.", Color::YELLOW);
        return;
    }

    for (size_t i = 0; i < lines.size(); i++)
    {
        printCentered(to_string(i + 1) + ". " + lines[i]);
    }
}

void showBeforeAfter(const vector<string> &before, const Document &doc)
{
    cout << "\n";
    printCentered("<<<<<<<<< BEFORE >>>>>>>>>", Color::CYAN + Color::BOLD);
    displaySnapshot(before);

    cout << "\n";
    printCentered("<<<<<<<<< AFTER >>>>>>>>>", Color::GREEN + Color::BOLD);
    displaySnapshot(captureDocument(doc));
}

void displayChoice(int choice)
{
    string action = "Invalid Option";

    if (choice >= 1 && choice <= (int)MENU_LABELS.size())
    {
        action = MENU_LABELS[choice - 1];
    }

    cout << "\n";
    printCentered(">>> You chose: " + action, Color::CYAN + Color::BOLD);
}

// ---------------------------------------------------------------
// File helpers
// ---------------------------------------------------------------

// All .txt files in the folder the program runs from
vector<string> listTextFiles()
{
    vector<string> files;

    try
    {
        for (const auto &entry : fs::directory_iterator(fs::current_path()))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".txt")
            {
                files.push_back(entry.path().filename().string());
            }
        }
    }
    catch (...)
    {
        // if the folder can't be read, just show an empty list
    }

    sort(files.begin(), files.end());
    return files;
}

void printFileList(const vector<string> &files)
{
    cout << Color::CYAN << Color::BOLD
         << "\nFiles in this folder:\n"
         << Color::RESET;

    if (files.empty())
    {
        cout << Color::GRAY << "  (no .txt files found yet)\n"
             << Color::RESET;
        return;
    }

    for (size_t i = 0; i < files.size(); i++)
    {
        cout << "  " << Color::GREEN << i + 1 << "." << Color::RESET
             << " " << files[i] << '\n';
    }
}

// If the user typed "notes", turn it into "notes.txt"
string addExtension(const string &name)
{
    if (name.find('.') == string::npos)
    {
        return name + ".txt";
    }
    return name;
}

bool isAllDigits(const string &s)
{
    if (s.empty())
        return false;

    for (char c : s)
    {
        if (c < '0' || c > '9')
            return false;
    }
    return true;
}

// Ask for a file name. The user can type a name or a number from the list.
// Returns "" if the user typed nothing.
string askFileName(const string &prompt, const vector<string> &files)
{
    string input;

    cout << Color::YELLOW << prompt << Color::RESET;

    if (!getline(cin, input))
    {
        return "";
    }

    if (input.empty())
    {
        return "";
    }

    if (isAllDigits(input))
    {
        size_t number = stoul(input);

        if (number >= 1 && number <= files.size())
        {
            return files[number - 1];
        }
    }

    return addExtension(input);
}

bool fileExists(const string &name)
{
    ifstream file(name);
    return file.good();
}

bool askYesNo(const string &question)
{
    string answer;

    while (true)
    {
        cout << Color::YELLOW << question << " (y/n): " << Color::RESET;

        if (!getline(cin, answer))
        {
            return false;
        }

        if (answer == "y" || answer == "Y")
            return true;
        if (answer == "n" || answer == "N")
            return false;

        cout << Color::RED << "Please enter y or n.\n"
             << Color::RESET;
    }
}

// ---------------------------------------------------------------
// Interactive file operations. FileManager performs the actual I/O;
// these functions handle prompts and messages for the terminal UI.
// ---------------------------------------------------------------

bool newDocument(Document &doc, UndoManager &undo, FileManager &files)
{
    vector<string> availableFiles = listTextFiles();
    printFileList(availableFiles);
    cout << '\n';

    string name = askFileName(
        "Name your new document (e.g. notes.txt), Enter to cancel: ", {});
    if (name.empty())
    {
        cout << Color::YELLOW << "New document cancelled.\n" << Color::RESET;
        return false;
    }

    if (fileExists(name))
    {
        cout << Color::RED << "'" << name << "' already exists.\n" << Color::RESET;
        if (!askYesNo("Start an empty document with this name (the old file is kept until you save)?"))
        {
            cout << Color::YELLOW << "New document cancelled.\n" << Color::RESET;
            return false;
        }
    }

    files.newDocument(doc, name);
    undo.clear();
    cout << Color::GREEN << "\nNew document '" << name
         << "' started.\n"
         << "It is written to disk only when you choose 'Save File'.\n"
         << Color::RESET;
    return true;
}

bool openFile(Document &doc, UndoManager &undo, FileManager &files)
{
    vector<string> availableFiles = listTextFiles();
    printFileList(availableFiles);
    cout << '\n';

    string filename = askFileName(
        "Type a file name or number to open, Enter to cancel: ", availableFiles);
    if (filename.empty())
    {
        cout << Color::YELLOW << "Open cancelled.\n" << Color::RESET;
        return false;
    }

    error_code ec;
    if (!fs::is_regular_file(filename, ec))
    {
        cout << Color::RED << "Error: '" << filename
             << "' does not exist or is not a regular file!\n" << Color::RESET;
        return false;
    }

    if (!files.openFile(filename, doc))
    {
        cout << Color::RED << "Error: Could not read '" << filename
             << "'. The current document was not changed.\n" << Color::RESET;
        return false;
    }

    undo.clear();
    cout << Color::GREEN << "\nFile opened successfully!\n"
         << "Now editing: " << files.getFileName() << '\n'
         << "Your changes will be saved into this file when you choose 'Save File'.\n"
         << Color::RESET;
    return true;
}

bool saveAs(Document &doc, FileManager &files)
{
    vector<string> availableFiles = listTextFiles();
    printFileList(availableFiles);
    cout << '\n';

    string filename = askFileName(
        "Type a new name, or a number to overwrite a file, Enter to cancel: ",
        availableFiles);
    if (filename.empty())
    {
        cout << Color::YELLOW << "Save As cancelled.\n" << Color::RESET;
        return false;
    }

    if (filename != files.getFileName() && fileExists(filename))
    {
        cout << Color::RED << "'" << filename << "' already exists.\n" << Color::RESET;
        if (!askYesNo("Overwrite it?"))
        {
            cout << Color::YELLOW << "Save As cancelled.\n" << Color::RESET;
            return false;
        }
    }

    if (!files.saveAs(filename, doc))
    {
        cout << Color::RED << "Error: Could not save to '" << filename << "'.\n"
             << Color::RESET;
        return false;
    }

    cout << Color::GREEN << "File saved successfully as "
         << files.getFileName() << "!\n" << Color::RESET;
    return true;
}

bool saveFile(Document &doc, FileManager &files)
{
    if (!files.hasFileName())
    {
        cout << Color::YELLOW
             << "This document has no file name yet, so choose one:\n"
             << Color::RESET;
        return saveAs(doc, files);
    }

    if (!files.save(doc))
    {
        cout << Color::RED << "Error: Could not save '" << files.getFileName()
             << "'.\n" << Color::RESET;
        return false;
    }

    cout << Color::GREEN << "Saved to " << files.getFileName() << "!\n"
         << Color::RESET;
    return true;
}

bool confirmUnsavedChanges(bool modified)
{
    if (!modified)
    {
        return true;
    }

    string answer;

    while (true)
    {
        cout << Color::YELLOW
             << "You have unsaved changes. Continue without saving? (y/n): "
             << Color::RESET;

        if (!getline(cin, answer))
        {
            return false;
        }

        if (answer == "y" || answer == "Y")
        {
            return true;
        }
        else if (answer == "n" || answer == "N")
        {
            return false;
        }

        cout << Color::RED
             << "Invalid input! Please enter y or n.\n"
             << Color::RESET;
    }
}

// ---------------------------------------------------------------
// Small input helpers
// ---------------------------------------------------------------
string readText(const string &prompt)
{
    string text;

    cout << prompt;

    if (!getline(cin, text))
    {
        cout << "\nInput stream closed. Exiting Text Editor.\n";
        exit(0);
    }

    return text;
}

void printError(const string &message)
{
    cout << Color::RED << message << '\n' << Color::RESET;
}

// Every command that needs text in the document calls this first
bool requireNonEmpty(const Document &doc)
{
    if (doc.getLineCount() == 0)
    {
        printError("The document is empty. Add a line first.");
        return false;
    }
    return true;
}

MatchOptions askMatchOptions()
{
    MatchOptions options;
    options.wholeWord = askYesNo("Match whole words only");
    options.caseSensitive = askYesNo("Case sensitive");
    return options;
}

void printMatchList(const Document &doc,
                    const DynamicArray<SearchLocation> &matches)
{
    for (size_t k = 0; k < matches.size(); k++)
    {
        cout << "  " << Color::GREEN << k + 1 << "." << Color::RESET
             << " Line " << matches[k].line
             << ", column " << matches[k].column
             << "  ->  " << doc.getLine((int)matches[k].line - 1) << '\n';
    }
}

// Ask which of `total` matches the user means. Returns 0 if invalid.
int askWhichMatch(size_t total, const string &verb)
{
    if (total == 1)
        return 1;

    int pick = readInt("Which one do you want to " + verb + "? (1-" +
                       to_string(total) + "): ");

    if (pick < 1 || pick > (int)total)
    {
        printError("Invalid choice!");
        return 0;
    }
    return pick;
}

// Delete one word at a position (plus one neighbouring space)
// and push it onto the undo stack.
bool deleteWordAt(Document &doc, UndoManager &undo,
                  int line, size_t pos, size_t wordLen)
{
    string text = doc.getLine(line);

    int column = (int)pos;
    int length = (int)wordLen;
    size_t end = pos + wordLen;

    if (pos > 0 && text[pos - 1] == ' ')
    {
        column--;
        length++;
    }
    else if (end < text.size() && text[end] == ' ')
    {
        length++;
    }

    string deletedText = doc.getText(line, column, length);

    if (!doc.deleteText(line, column, length))
        return false;

    undo.recordDelete(line, column, deletedText);
    return true;
}

// ---------------------------------------------------------------
// Editing commands (one function per menu item)
// ---------------------------------------------------------------

// 5. INSERT A NEW LINE at any position (or at the end)
void cmdInsertLine(Document &doc, UndoManager &undo, FileManager &files)
{
    int n = doc.getLineCount();

    int pos = readInt("Insert at line number (1-" + to_string(n + 1) +
                      ", " + to_string(n + 1) + " = end of document): ");

    if (pos < 1 || pos > n + 1)
    {
        printError("Invalid line number!");
        return;
    }

    string text = readText("Enter your text (blank = empty line): ");

    vector<string> before = captureDocument(doc);

    if (doc.insertLine(pos - 1, text))
    {
        undo.recordLineInsert(pos - 1, text);
        files.markModified();

        cout << Color::GREEN << "Line inserted successfully!\n"
             << Color::RESET;
        showBeforeAfter(before, doc);
    }
    else
    {
        printError("Could not insert line.");
    }
}

// 7. SEARCH
void cmdSearch(const Document &doc)
{
    if (!requireNonEmpty(doc))
        return;

    string word = readText("Enter text to search: ");

    if (word.empty())
    {
        printError("Search text cannot be empty.");
        return;
    }

    MatchOptions options = askMatchOptions();
    DynamicArray<SearchLocation> results = Search::findAll(doc, word, options);

    if (results.empty())
    {
        printError("'" + word + "' was not found in the document.");
        return;
    }

    cout << Color::GREEN << "\nFound " << results.size()
         << " occurrence(s):\n" << Color::RESET;
    printMatchList(doc, results);
}

// 8. REPLACE ONE (user chooses which occurrence)
void cmdReplaceOne(Document &doc, UndoManager &undo, FileManager &files)
{
    if (!requireNonEmpty(doc))
        return;

    string oldWord = readText("Enter text to replace: ");

    if (oldWord.empty())
    {
        printError("The text to replace cannot be empty.");
        return;
    }

    string newWord = readText("Enter replacement (blank = delete it): ");
    MatchOptions options = askMatchOptions();

    DynamicArray<SearchLocation> matches = Search::findAll(doc, oldWord, options);

    if (matches.empty())
    {
        printError("'" + oldWord + "' was not found in the document.");
        return;
    }

    cout << Color::CYAN << Color::BOLD << "\nFound " << matches.size()
         << " occurrence(s):\n" << Color::RESET;
    printMatchList(doc, matches);

    int pick = askWhichMatch(matches.size(), "replace");
    if (pick == 0)
        return;

    vector<string> before = captureDocument(doc);

    if (Replace::replaceNth(doc, oldWord, newWord, (size_t)pick, &undo, options))
    {
        files.markModified();
        cout << Color::GREEN << "Occurrence replaced successfully!\n"
             << Color::RESET;
        showBeforeAfter(before, doc);
    }
    else
    {
        printError("Nothing changed (the text is already the replacement).");
    }
}

// 9. REPLACE ALL
void cmdReplaceAll(Document &doc, UndoManager &undo, FileManager &files)
{
    if (!requireNonEmpty(doc))
        return;

    string oldWord = readText("Enter text to replace: ");

    if (oldWord.empty())
    {
        printError("The text to replace cannot be empty.");
        return;
    }

    string newWord = readText("Enter replacement (blank = delete it): ");
    MatchOptions options = askMatchOptions();

    vector<string> before = captureDocument(doc);
    size_t count = Replace::replaceAll(doc, oldWord, newWord, &undo, options);

    if (count > 0)
    {
        files.markModified();
        cout << Color::GREEN << "\n" << count
             << " replacement(s) made! (one Undo reverses all of them)\n"
             << Color::RESET;
        showBeforeAfter(before, doc);
    }
    else
    {
        printError("No replacements made ('" + oldWord + "' not found).");
    }
}

// 10. INSERT TEXT inside a line
void cmdInsertText(Document &doc, UndoManager &undo, FileManager &files)
{
    if (!requireNonEmpty(doc))
        return;

    int line = readInt("Enter line number: ");
    int column = readInt("Enter column number: ");
    string text = readText("Enter text to insert: ");

    if (text.empty())
    {
        printError("Nothing to insert.");
        return;
    }

    vector<string> before = captureDocument(doc);

    // Convert 1-based user positions to 0-based indices
    if (doc.insertText(line - 1, column - 1, text))
    {
        undo.recordInsert(line - 1, column - 1, text);
        files.markModified();

        showBeforeAfter(before, doc);
        cout << Color::GREEN << "Text inserted successfully!\n"
             << Color::RESET;
    }
    else
    {
        printError("Could not insert text. Check the line and column "
                   "(column may be 1 to line length + 1).");
    }
}

// 11a. DELETE a character or a range of characters in one line
void deleteCharacters(Document &doc, UndoManager &undo, FileManager &files)
{
    int line = readInt("Enter line number: ");
    int column = readInt("Enter starting column: ");
    int length = readInt("How many characters to delete (1 = a single character): ");

    if (length <= 0)
    {
        printError("The number of characters must be at least 1.");
        return;
    }

    string removed = doc.getText(line - 1, column - 1, length);

    if (removed.empty())
    {
        printError("Invalid range. The characters must exist on that line.");
        return;
    }

    vector<string> before = captureDocument(doc);

    if (doc.deleteText(line - 1, column - 1, length))
    {
        undo.recordDelete(line - 1, column - 1, removed);
        files.markModified();

        showBeforeAfter(before, doc);
        cout << Color::GREEN << length << " character(s) deleted!\n"
             << Color::RESET;
    }
    else
    {
        printError("Could not delete the text.");
    }
}

// 11b / 11c. DELETE one chosen occurrence of a word, or every occurrence
void deleteWord(Document &doc, UndoManager &undo, FileManager &files, bool all)
{
    string word = readText("Enter the word to delete: ");

    if (word.empty())
    {
        printError("No word entered.");
        return;
    }

    MatchOptions options;
    options.wholeWord = true;
    options.caseSensitive = askYesNo("Case sensitive");

    DynamicArray<SearchLocation> matches = Search::findAll(doc, word, options);

    if (matches.empty())
    {
        printError("That word is not in the document.");
        return;
    }

    vector<string> before = captureDocument(doc);
    int deleted = 0;

    if (!all)
    {
        cout << Color::CYAN << Color::BOLD << "\nFound " << matches.size()
             << " occurrence(s):\n" << Color::RESET;
        printMatchList(doc, matches);

        int pick = askWhichMatch(matches.size(), "delete");
        if (pick == 0)
            return;

        const SearchLocation &m = matches[pick - 1];

        if (deleteWordAt(doc, undo, (int)m.line - 1,
                         m.column - 1, word.size()))
            deleted = 1;
    }
    else
    {
        // Right-to-left so earlier positions stay valid;
        // grouped so ONE undo restores every deleted occurrence.
        undo.beginGroup();

        for (size_t k = matches.size(); k > 0; k--)
        {
            const SearchLocation &m = matches[k - 1];

            if (deleteWordAt(doc, undo, (int)m.line - 1,
                             m.column - 1, word.size()))
                deleted++;
        }

        undo.endGroup();
    }

    if (deleted > 0)
    {
        files.markModified();
        showBeforeAfter(before, doc);
        cout << Color::GREEN << deleted << " occurrence(s) deleted!\n"
             << Color::RESET;
    }
    else
    {
        printError("Nothing was deleted.");
    }
}

// 11. DELETE TEXT (sub-menu)
void cmdDeleteText(Document &doc, UndoManager &undo, FileManager &files)
{
    if (!requireNonEmpty(doc))
        return;

    cout << "  1. Delete a character / range of characters in a line\n";
    cout << "  2. Delete a word (choose which occurrence)\n";
    cout << "  3. Delete ALL occurrences of a word\n";
    int mode = readInt("Choose delete mode: ");

    switch (mode)
    {
    case 1: deleteCharacters(doc, undo, files); break;
    case 2: deleteWord(doc, undo, files, false); break;
    case 3: deleteWord(doc, undo, files, true); break;
    default: printError("Invalid mode!");
    }
}

// 12. DELETE a line or a range of lines
void cmdDeleteLines(Document &doc, UndoManager &undo, FileManager &files)
{
    if (!requireNonEmpty(doc))
        return;

    int n = doc.getLineCount();

    int from = readInt("First line to delete: ");
    int to = readInt("Last line to delete (same number = one line): ");

    if (from < 1 || to < from || to > n)
    {
        printError("Invalid range. Lines must be between 1 and " +
                   to_string(n) + ", and the first cannot be after the last.");
        return;
    }

    vector<string> before = captureDocument(doc);
    int deleted = 0;

    // Bottom-up so the line numbers above stay valid; grouped so one
    // Undo brings the whole range back.
    undo.beginGroup();

    for (int i = to - 1; i >= from - 1; i--)
    {
        string text = doc.getLine(i);

        if (doc.deleteLine(i))
        {
            undo.recordLineDelete(i, text);
            deleted++;
        }
    }

    undo.endGroup();

    files.markModified();
    showBeforeAfter(before, doc);
    cout << Color::GREEN << deleted << " line(s) deleted!\n" << Color::RESET;
}

// 13. UNDO
void cmdUndo(Document &doc, UndoManager &undo, FileManager &files)
{
    if (!undo.canUndo())
    {
        cout << Color::YELLOW << "Nothing to undo.\n" << Color::RESET;
        return;
    }

    vector<string> before = captureDocument(doc);

    if (undo.undo(doc))
    {
        files.markModified();
        cout << Color::GREEN << undo.lastMessage() << '\n' << Color::RESET;
        showBeforeAfter(before, doc);
    }
    else
    {
        printError("Undo failed: " + undo.lastMessage());
    }
}

// ---------------------------------------------------------------
// main
// ---------------------------------------------------------------
int main()
{
#ifdef _WIN32
    system("chcp 65001 > nul"); // UTF-8 so block letters and boxes display
#endif

    Document doc;
    UndoManager undo;

    FileManager files;
    int choice;

    do
    {
        displayHeader(files.getFileName(), files.isModified());
        displayMenu();
        displayStatusBar(doc, undo, files.getFileName(), files.isModified());

        choice = readInt("Enter your choice: ", true);
        displayChoice(choice);

        cout << "\n";

        switch (choice)
        {
        // ===================== FILE =====================
        case 1:
            if (confirmUnsavedChanges(files.isModified()))
                newDocument(doc, undo, files);
            break;

        case 2:
            if (confirmUnsavedChanges(files.isModified()))
                openFile(doc, undo, files);
            break;

        case 3:
            saveFile(doc, files);
            break;

        case 4:
            saveAs(doc, files);
            break;

        // ===================== FILE MANAGEMENT =====================
        case 5:  displayDocument(doc);                break;

        // ===================== TEXT EDITING =====================
        case 6:  cmdInsertLine(doc, undo, files);     break;
        case 7:  cmdInsertText(doc, undo, files);     break;
        case 8:  cmdDeleteText(doc, undo, files);     break;
        case 9:  cmdDeleteLines(doc, undo, files);    break;

        // ===================== SEARCH & REPLACE =====================
        case 10: cmdSearch(doc);                      break;
        case 11: cmdReplaceOne(doc, undo, files);     break;
        case 12: cmdReplaceAll(doc, undo, files);     break;

        // ===================== UNDO =====================
        case 13: cmdUndo(doc, undo, files);           break;

        // ===================== EXIT =====================
        case 14:
            if (confirmUnsavedChanges(files.isModified()))
            {
                cout << Color::GREEN
                     << "\nThank you for using Text Editor!\n"
                     << Color::RESET;
            }
            else
            {
                choice = 0; // user said "no", so stay in the loop
            }
            break;

        default:
            printError("Invalid choice! Select 1-14.");
        }

    } while (choice != EXIT_CHOICE);

    return 0;
}