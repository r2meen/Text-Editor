#include <iostream>
#include <string>
#include <limits>
#include <sstream>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <filesystem>

#include "include/Document.h"
#include "include/Search.h"
#include "include/Replace.h"
#include "include/UndoManager.h"

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
    "New Document",             // 1
    "Open File",                // 2
    "Save File",                // 3
    "Save As",                  // 4
    "Add a new line",           // 5
    "Display document",         // 6
    "Search for a word",        // 7
    "Replace first occurrence", // 8
    "Replace all occurrences",  // 9
    "Insert text into a line",  // 10
    "Delete text from a line",  // 11
    "Undo last operation",      // 12
    "Document statistics",      // 13
    "Exit"                      // 14
};

const int FILE_COUNT = 4;
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

    printSection("Editing");
    for (int i = FILE_COUNT; i < EXIT_CHOICE - 1; i++)
    {
        printItem(i + 1, MENU_LABELS[i]);
    }

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

    string left = " " + name + " | Lines: " + to_string(doc.getLineCount()) + " | Chars: " + to_string(doc.countCharacters()) + " | Undo: " + to_string(undo.historySize());

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
    cout << Color::CYAN << Color::BOLD;
    cout << "\n-------------- DOCUMENT -------------------\n";
    cout << Color::RESET;

    if (doc.getLineCount() == 0)
    {
        cout << Color::YELLOW
             << "The document is empty.\n"
             << Color::RESET;
        return;
    }

    for (int i = 0; i < doc.getLineCount(); i++)
    {
        cout << Color::YELLOW << i + 1 << ". "
             << Color::RESET;

        cout << doc.getLine(i) << '\n';
    }

    cout << Color::CYAN
         << "--------------------------------------------\n"
         << Color::RESET;
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
        cout << Color::YELLOW << "Document is empty.\n"
             << Color::RESET;
        return;
    }

    for (size_t i = 0; i < lines.size(); i++)
    {
        cout << Color::YELLOW << i + 1 << ". "
             << Color::RESET << lines[i] << '\n';
    }
}

void showBeforeAfter(const vector<string> &before, const Document &doc)
{
    cout << Color::CYAN << Color::BOLD;
    cout << "\n========== BEFORE ==========\n";
    cout << Color::RESET;

    displaySnapshot(before);

    cout << Color::GREEN << Color::BOLD;
    cout << "\n========== AFTER ===========\n";
    cout << Color::RESET;

    displayDocument(doc);
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

// Write every line of the document into a file
bool writeDocument(const Document &doc, const string &filename)
{
    ofstream file(filename);

    if (!file.is_open())
    {
        cout << Color::RED << "Error: Could not save file!\n"
             << Color::RESET;
        return false;
    }

    for (int i = 0; i < doc.getLineCount(); i++)
    {
        file << doc.getLine(i) << '\n';
    }

    file.flush();

    if (!file)
    {
        cout << Color::RED << "Error: Problem writing to file!\n"
             << Color::RESET;
        return false;
    }

    return true;
}

// ---------------------------------------------------------------
// File operations
// ---------------------------------------------------------------

// 1. NEW DOCUMENT: ask for a name, create the file, start editing it
bool newDocument(Document &doc, UndoManager &undo, string &currentFile)
{
    vector<string> files = listTextFiles();
    printFileList(files);

    cout << '\n';
    string name = askFileName(
        "Name your new document (e.g. notes.txt), Enter to cancel: ", {});

    if (name.empty())
    {
        cout << Color::YELLOW << "New document cancelled.\n"
             << Color::RESET;
        return false;
    }

    if (fileExists(name))
    {
        cout << Color::RED << "'" << name << "' already exists.\n"
             << Color::RESET;

        if (!askYesNo("Replace it with an empty document?"))
        {
            cout << Color::YELLOW << "New document cancelled.\n"
                 << Color::RESET;
            return false;
        }
    }

    // Create the empty file on disk
    ofstream file(name);

    if (!file.is_open())
    {
        cout << Color::RED << "Error: Could not create the file!\n"
             << Color::RESET;
        return false;
    }

    file.close();

    doc.clear();
    undo.clear();
    currentFile = name;

    cout << Color::GREEN << "\nDocument '" << name
         << "' created successfully!\n"
         << "You are now editing it. Choose 'Save File' to save your work.\n"
         << Color::RESET;

    return true;
}

// 2. OPEN FILE: show the files, load the chosen one, start editing it
bool openFile(Document &doc, UndoManager &undo, string &currentFile)
{
    vector<string> files = listTextFiles();
    printFileList(files);

    cout << '\n';
    string filename = askFileName(
        "Type a file name or number to open, Enter to cancel: ", files);

    if (filename.empty())
    {
        cout << Color::YELLOW << "Open cancelled.\n"
             << Color::RESET;
        return false;
    }

    ifstream file(filename);

    if (!file.is_open())
    {
        cout << Color::RED << "Error: Could not open '" << filename
             << "'!\n"
             << Color::RESET;
        return false;
    }

    vector<string> lines;
    string line;

    while (getline(file, line))
    {
        lines.push_back(line);
    }

    if (file.bad())
    {
        cout << Color::RED << "Error: Problem reading the file!\n"
             << Color::RESET;
        return false;
    }

    doc.clear();
    undo.clear();

    for (const string &text : lines)
    {
        doc.appendLine(text);
    }

    currentFile = filename;

    cout << Color::GREEN << "\nFile opened successfully!\n"
         << "Now editing: " << currentFile << '\n'
         << "Your changes will be saved into this file when you choose "
            "'Save File'.\n"
         << Color::RESET;

    return true;
}

// 4. SAVE AS: show the files, pick one (or type a new name), save into it
bool saveAs(Document &doc, string &currentFile)
{
    vector<string> files = listTextFiles();
    printFileList(files);

    cout << '\n';
    string filename = askFileName(
        "Type a new name, or a number to overwrite a file, Enter to cancel: ",
        files);

    if (filename.empty())
    {
        cout << Color::YELLOW << "Save As cancelled.\n"
             << Color::RESET;
        return false;
    }

    // Warn before overwriting a different existing file
    if (filename != currentFile && fileExists(filename))
    {
        cout << Color::RED << "'" << filename << "' already exists.\n"
             << Color::RESET;

        if (!askYesNo("Overwrite it?"))
        {
            cout << Color::YELLOW << "Save As cancelled.\n"
                 << Color::RESET;
            return false;
        }
    }

    if (!writeDocument(doc, filename))
    {
        return false;
    }

    currentFile = filename;

    cout << Color::GREEN << "File saved successfully as "
         << currentFile << "!\n"
         << Color::RESET;

    return true;
}

// 3. SAVE FILE: save into the file we are already editing
bool saveFile(Document &doc, string &currentFile)
{
    if (currentFile.empty())
    {
        cout << Color::YELLOW
             << "This document has no file name yet, so choose one:\n"
             << Color::RESET;
        return saveAs(doc, currentFile);
    }

    if (!writeDocument(doc, currentFile))
    {
        return false;
    }

    cout << Color::GREEN << "Saved to " << currentFile << "!\n"
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

// Find the word as a whole word (not inside another word).
// Returns the position, or string::npos if there is none.
size_t findWholeWord(const string &text, const string &word, size_t start)
{
    size_t p = start;

    while ((p = text.find(word, p)) != string::npos)
    {
        size_t e = p + word.size();

        bool startOk = (p == 0 || !isalnum((unsigned char)text[p - 1]));
        bool endOk = (e >= text.size() || !isalnum((unsigned char)text[e]));

        if (startOk && endOk)
            return p;

        p++;
    }

    return string::npos;
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
// main
// ---------------------------------------------------------------
int main()
{
#ifdef _WIN32
    system("chcp 65001 > nul"); // UTF-8 so block letters and boxes display
#endif

    Document doc;
    UndoManager undo;

    string currentFile = "";
    bool modified = false;
    int choice;

    do
    {
        displayHeader(currentFile, modified);
        displayMenu();
        displayStatusBar(doc, undo, currentFile, modified);

        choice = readInt("Enter your choice: ", true);
        displayChoice(choice);

        switch (choice)
        {
        // ===================== FILE =====================

        // 1. NEW DOCUMENT
        case 1:
        {
            if (confirmUnsavedChanges(modified))
            {
                if (newDocument(doc, undo, currentFile))
                {
                    modified = false;
                }
            }
            break;
        }

        // 2. OPEN FILE
        case 2:
        {
            if (confirmUnsavedChanges(modified))
            {
                if (openFile(doc, undo, currentFile))
                {
                    modified = false;
                }
            }
            break;
        }

        // 3. SAVE FILE
        case 3:
        {
            if (saveFile(doc, currentFile))
            {
                modified = false;
            }
            break;
        }

        // 4. SAVE AS
        case 4:
        {
            if (saveAs(doc, currentFile))
            {
                modified = false;
            }
            break;
        }

        // ===================== EDIT =====================

        // 5. ADD LINE
        case 5:
        {
            string text;

            cout << "Enter your text: ";
            getline(cin, text);

            vector<string> before = captureDocument(doc);
            int index = doc.getLineCount();

            if (doc.appendLine(text))
            {
                undo.recordLineInsert(index, text);
                modified = true;

                cout << Color::GREEN
                     << "Line added successfully!\n"
                     << Color::RESET;

                showBeforeAfter(before, doc);
            }
            else
            {
                cout << Color::RED
                     << "Could not add line.\n"
                     << Color::RESET;
            }

            break;
        }

        // 6. DISPLAY DOCUMENT
        case 6:
        {
            displayDocument(doc);
            break;
        }

        // 7. SEARCH
        case 7:
        {
            string word;

            cout << "Enter word to search: ";
            getline(cin, word);

            auto results = Search::findAll(doc, word);

            if (results.empty())
            {
                cout << Color::RED
                     << "Word not found!\n"
                     << Color::RESET;
            }
            else
            {
                cout << Color::GREEN
                     << "\nFound " << results.size()
                     << " occurrence(s):\n"
                     << Color::RESET;

                for (const auto &result : results)
                {
                    cout << "Line: " << result.line
                         << " | Column: " << result.column
                         << '\n';
                }
            }

            break;
        }

        // 8. REPLACE FIRST
        case 8:
        {
            string oldWord, newWord;

            cout << "Enter word to replace: ";
            getline(cin, oldWord);

            cout << "Enter replacement: ";
            getline(cin, newWord);

            vector<string> before = captureDocument(doc);

            if (Replace::replaceFirst(doc, oldWord, newWord, &undo))
            {
                modified = true;

                cout << Color::GREEN
                     << "First occurrence replaced successfully!\n"
                     << Color::RESET;

                showBeforeAfter(before, doc);
            }
            else
            {
                cout << Color::RED
                     << "Word not found or replacement failed.\n"
                     << Color::RESET;
            }

            break;
        }

        // 9. REPLACE ALL
        case 9:
        {
            string oldWord, newWord;

            cout << "Enter word to replace: ";
            getline(cin, oldWord);

            cout << "Enter replacement: ";
            getline(cin, newWord);

            vector<string> before = captureDocument(doc);

            size_t count = Replace::replaceAll(doc, oldWord, newWord, &undo);

            if (count > 0)
            {
                modified = true;

                cout << Color::GREEN
                     << "\n"
                     << count << " replacement(s) made!\n"
                     << Color::RESET;

                showBeforeAfter(before, doc);
            }
            else
            {
                cout << Color::RED
                     << "No replacements made.\n"
                     << Color::RESET;
            }

            break;
        }

        // 10. INSERT TEXT
        case 10:
        {
            vector<string> before = captureDocument(doc);

            if (doc.getLineCount() == 0)
            {
                cout << Color::RED
                     << "Document is empty. Add a line first.\n"
                     << Color::RESET;
                break;
            }

            int line = readInt("Enter line number: ");
            int column = readInt("Enter column number: ");

            string text;

            cout << "Enter text to insert: ";
            getline(cin, text);

            // Convert 1-based user positions to 0-based indices
            line--;
            column--;

            if (line < 0 || column < 0)
            {
                cout << Color::RED
                     << "Invalid position!\n"
                     << Color::RESET;
                break;
            }

            if (doc.insertText(line, column, text))
            {
                modified = true;
                undo.recordInsert(line, column, text);
                showBeforeAfter(before, doc);

                cout << Color::GREEN
                     << "Text inserted successfully!\n"
                     << Color::RESET;
            }
            else
            {
                cout << Color::RED
                     << "Could not insert text. Check the position.\n"
                     << Color::RESET;
            }

            break;
        }

        // 11. DELETE A WORD
        case 11:
        {
            vector<string> before = captureDocument(doc);

            if (doc.getLineCount() == 0)
            {
                cout << Color::RED << "Document is empty.\n"
                     << Color::RESET;
                break;
            }

            cout << "  1. Choose which occurrence to delete\n";
            cout << "  2. Delete all occurrences\n";
            int mode = readInt("Choose delete mode: ");

            if (mode != 1 && mode != 2)
            {
                cout << Color::RED << "Invalid mode!\n" << Color::RESET;
                break;
            }

            string word;
            cout << "Enter the word to delete: ";
            getline(cin, word);

            if (word.empty())
            {
                cout << Color::RED << "No word entered.\n"
                     << Color::RESET;
                break;
            }

            int deleted = 0;

            if (mode == 1)
            {
                // Collect every match first
                vector<int> matchLine;
                vector<size_t> matchPos;

                for (int i = 0; i < doc.getLineCount(); i++)
                {
                    string text = doc.getLine(i);
                    size_t p = 0;

                    while ((p = findWholeWord(text, word, p)) != string::npos)
                    {
                        matchLine.push_back(i);
                        matchPos.push_back(p);
                        p += word.size();
                    }
                }

                if (matchLine.empty())
                {
                    cout << Color::RED << "That word is not in the document.\n"
                         << Color::RESET;
                    break;
                }

                cout << Color::CYAN << Color::BOLD
                     << "\nFound " << matchLine.size()
                     << " occurrence(s):\n" << Color::RESET;

                for (size_t k = 0; k < matchLine.size(); k++)
                {
                    cout << "  " << Color::GREEN << k + 1 << "."
                         << Color::RESET
                         << " Line " << matchLine[k] + 1
                         << ", column " << matchPos[k] + 1
                         << "  ->  " << doc.getLine(matchLine[k]) << '\n';
                }

                int pick = readInt("Which one do you want to delete? ");

                if (pick < 1 || pick > (int)matchLine.size())
                {
                    cout << Color::RED << "Invalid choice!\n"
                         << Color::RESET;
                    break;
                }

                if (deleteWordAt(doc, undo, matchLine[pick - 1],
                                 matchPos[pick - 1], word.size()))
                {
                    deleted = 1;
                }
            }
            else
            {
                // All occurrences, bottom-up and right-to-left,
                // grouped so one Undo restores everything
                undo.beginGroup();

                for (int i = doc.getLineCount() - 1; i >= 0; i--)
                {
                    vector<size_t> matches;
                    string text = doc.getLine(i);
                    size_t p = 0;

                    while ((p = findWholeWord(text, word, p)) != string::npos)
                    {
                        matches.push_back(p);
                        p += word.size();
                    }

                    for (int k = (int)matches.size() - 1; k >= 0; k--)
                    {
                        if (deleteWordAt(doc, undo, i, matches[k], word.size()))
                            deleted++;
                    }
                }

                undo.endGroup();
            }

            if (deleted > 0)
            {
                modified = true;
                showBeforeAfter(before, doc);

                cout << Color::GREEN << deleted << " occurrence(s) deleted!\n"
                     << Color::RESET;
            }
            else
            {
                cout << Color::RED << "That word is not in the document.\n"
                     << Color::RESET;
            }

            break;
        }

        // 12. UNDO
        case 12:
        {
            if (undo.canUndo())
            {
                vector<string> before = captureDocument(doc);

                if (undo.undo(doc))
                {
                    modified = true;

                    cout << Color::GREEN
                         << "Last operation undone successfully!\n"
                         << Color::RESET;

                    showBeforeAfter(before, doc);
                }
                else
                {
                    cout << Color::RED
                         << "Undo failed: "
                         << undo.lastMessage() << '\n'
                         << Color::RESET;
                }
            }
            else
            {
                cout << Color::YELLOW
                     << "Nothing to undo.\n"
                     << Color::RESET;
            }

            break;
        }

        // 13. STATISTICS
        case 13:
        {
            cout << Color::CYAN << Color::BOLD;
            cout << "\n---------- DOCUMENT STATISTICS ------------\n";
            cout << Color::RESET;

            cout << "Current file: "
                 << (currentFile.empty() ? "Untitled (not saved yet)"
                                         : currentFile)
                 << '\n';

            cout << "Total lines: "
                 << doc.getLineCount() << '\n';

            cout << "Total characters: "
                 << doc.countCharacters() << '\n';

            cout << "Undo history: "
                 << undo.historySize() << " operation(s)\n";

            cout << Color::CYAN
                 << "--------------------------------------------\n"
                 << Color::RESET;

            break;
        }

        // ===================== EXIT =====================

        // 14. EXIT
        case 14:
        {
            if (confirmUnsavedChanges(modified))
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
        }

        default:
        {
            cout << Color::RED
                 << "Invalid choice! Select 1-14.\n"
                 << Color::RESET;
        }
        }

    } while (choice != EXIT_CHOICE);

    return 0;
}