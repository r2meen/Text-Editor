#include <iostream>
#include <string>
#include <limits>
#include <sstream>
#include <vector>

#include "include/Document.h"
#include "include/Search.h"
#include "include/Replace.h"
#include "include/UndoManager.h"

using namespace std;

// Terminal colours
namespace Color
{
    const string RESET = "\033[0m";
    const string RED = "\033[31m";
    const string GREEN = "\033[32m";
    const string YELLOW = "\033[33m";
    const string BLUE = "\033[34m";
    const string CYAN = "\033[36m";
    const string WHITE = "\033[97m";
    const string BOLD = "\033[1m";
}

// Read an integer safely
int readInt(string prompt)
{
    string input;
    int number;

    while (true)
    {
        cout << Color::YELLOW << prompt << Color::RESET;
        getline(cin, input);

        stringstream ss(input);

        if (ss >> number && ss.eof())
        {
            return number;
        }

        cout << Color::RED
             << "Invalid input! Please enter a number.\n"
             << Color::RESET;
    }
}

// Display application heading
void displayHeader()
{
    cout << "\n";
    cout << Color::CYAN << Color::BOLD;
    cout << "============================================\n";
    cout << "             TEXT EDITOR                    \n";
    cout << "============================================\n";
    cout << Color::RESET;
}

// Display menu
void displayMenu()
{
    cout << Color::BLUE << Color::BOLD;
    cout << "\n--------------- MAIN MENU -----------------\n";
    cout << Color::RESET;

    cout << Color::GREEN << "1." << Color::RESET
         << " Add a new line\n";

    cout << Color::GREEN << "2." << Color::RESET
         << " Display document\n";

    cout << Color::GREEN << "3." << Color::RESET
         << " Search for a word\n";

    cout << Color::GREEN << "4." << Color::RESET
         << " Replace first occurrence\n";

    cout << Color::GREEN << "5." << Color::RESET
         << " Replace all occurrences\n";

    cout << Color::GREEN << "6." << Color::RESET
         << " Insert text into a line\n";

    cout << Color::GREEN << "7." << Color::RESET
         << " Delete text from a line\n";

    cout << Color::GREEN << "8." << Color::RESET
         << " Undo last operation\n";

    cout << Color::GREEN << "9." << Color::RESET
         << " Document statistics\n";

    cout << Color::RED << "10." << Color::RESET
         << " Exit\n";

    cout << Color::BLUE << "--------------------------------------------\n";
    cout << Color::RESET;
}

// Display document
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

// Save the current document state
vector<string> captureDocument(const Document &doc)
{
    vector<string> lines;

    for (int i = 0; i < doc.getLineCount(); i++)
    {
        lines.push_back(doc.getLine(i));
    }

    return lines;
}

// Display a saved document state
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

// Show before and after
void showBeforeAfter(
    const vector<string> &before,
    const Document &doc)
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

// Announce the selected menu option
void displayChoice(int choice)
{
    string action;

    switch (choice)
    {
    case 1:
        action = "Add a New Line";
        break;
    case 2:
        action = "Display Document";
        break;
    case 3:
        action = "Search for a Word";
        break;
    case 4:
        action = "Replace First Occurrence";
        break;
    case 5:
        action = "Replace All Occurrences";
        break;
    case 6:
        action = "Insert Text";
        break;
    case 7:
        action = "Delete Text";
        break;
    case 8:
        action = "Undo Last Operation";
        break;
    case 9:
        action = "Document Statistics";
        break;
    case 10:
        action = "Exit";
        break;
    default:
        action = "Invalid Option";
    }

    cout << Color::CYAN << Color::BOLD;
    cout << "\n>>> You chose: " << action << "\n";
    cout << Color::RESET;
}

int main()
{

    Document doc;
    UndoManager undo;

    int choice;

    displayHeader();

    cout << Color::GREEN
         << "Welcome to your Text Editor!\n"
         << Color::RESET;

    do
    {
        displayMenu();

        choice = readInt("Enter your choice: ");
        displayChoice(choice);

        switch (choice)
        {

            // ADD LINE
        case 1:
        {
            string text;

            cout << "Enter your text: ";
            getline(cin, text);

            vector<string> before = captureDocument(doc);
            int index = doc.getLineCount();

            if (doc.appendLine(text))
            {
                undo.recordLineInsert(index, text);

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

        // DISPLAY DOCUMENT
        case 2:
        {
            displayDocument(doc);
            break;
        }

        // SEARCH
        case 3:
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

        // REPLACE FIRST
        case 4:
        {
            string oldWord, newWord;

            cout << "Enter word to replace: ";
            getline(cin, oldWord);

            cout << "Enter replacement: ";
            getline(cin, newWord);

            vector<string> before = captureDocument(doc);

            if (Replace::replaceFirst(doc, oldWord, newWord, &undo))
            {

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

            // REPLACE ALL
        case 5:
        {
            string oldWord, newWord;

            cout << "Enter word to replace: ";
            getline(cin, oldWord);

            cout << "Enter replacement: ";
            getline(cin, newWord);

            vector<string> before = captureDocument(doc);

            size_t count = Replace::replaceAll(
                doc, oldWord, newWord, &undo);

            if (count > 0)
            {
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
        // INSERT TEXT
        case 6:
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

        // DELETE TEXT
        case 7:
        {
            vector<string> before = captureDocument(doc);

            if (doc.getLineCount() == 0)
            {
                cout << Color::RED
                     << "Document is empty.\n"
                     << Color::RESET;
                break;
            }

            int line = readInt("Enter line number: ");
            int column = readInt("Enter starting column: ");
            int length = readInt("Enter number of characters to delete: ");

            line--;
            column--;

            if (line < 0 || column < 0 || length <= 0)
            {
                cout << Color::RED
                     << "Invalid position or length!\n"
                     << Color::RESET;
                break;
            }

            string deletedText = doc.getText(line, column, length);

            if (deletedText.empty())
            {
                cout << Color::RED
                     << "Invalid deletion range!\n"
                     << Color::RESET;
                break;
            }

            if (doc.deleteText(line, column, length))
            {
                undo.recordDelete(line, column, deletedText);
                showBeforeAfter(before, doc);

                cout << Color::GREEN
                     << "Text deleted successfully!\n"
                     << Color::RESET;
            }
            else
            {
                cout << Color::RED
                     << "Could not delete text.\n"
                     << Color::RESET;
            }

            break;
        }

            // UNDO
        case 8:
        {
            if (undo.canUndo())
            {

                vector<string> before = captureDocument(doc);

                if (undo.undo(doc))
                {

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

        // STATISTICS
        case 9:
        {
            cout << Color::CYAN << Color::BOLD;
            cout << "\n---------- DOCUMENT STATISTICS ------------\n";
            cout << Color::RESET;

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

        // EXIT
        case 10:
        {
            cout << Color::GREEN
                 << "\nThank you for using Text Editor!\n"
                 << Color::RESET;
            break;
        }

        default:
        {
            cout << Color::RED
                 << "Invalid choice! Select 1-10.\n"
                 << Color::RESET;
        }
        }

    } while (choice != 10);

    return 0;
}
