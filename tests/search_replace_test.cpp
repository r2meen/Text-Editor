#include <iostream>
#include <string>
#include <vector>
#include "../include/Search.h"
#include "../include/Replace.h"

using namespace std;

int main()
{
    string document;
    string line;
    int choice;

    cout << "Enter document text." << endl;
    cout << "Type END on a separate line when finished." << endl;
    cout << "----------------------------------------" << endl;

    while (true)
    {
        getline(cin, line);

        if (line == "END")
        {
            break;
        }

        document += line;
        document += '\n';
    }

    cout << "\n1. Search" << endl;
    cout << "2. Replace" << endl;
    cout << "Enter choice: ";
    cin >> choice;
    cin.ignore();

    if (choice == 1)
    {
        string searchText;

        cout << "Enter text to search: ";
        getline(cin, searchText);

        vector<SearchLocation> locations =
            Search::findAll(document, searchText);

        cout << "\nOccurrences found: "
             << locations.size() << endl;

        if (locations.empty())
        {
            cout << "Text not found." << endl;
        }
        else
        {
            for (const SearchLocation& location : locations)
            {
                cout << "Line: " << location.line
                     << ", Column: " << location.column
                     << endl;
            }
        }
    }
    else if (choice == 2)
    {
        string searchText;
        string replaceText;

        cout << "Enter text to search: ";
        getline(cin, searchText);

        cout << "Enter replacement text: ";
        getline(cin, replaceText);

        size_t count =
            Replace::replaceAll(
                document,
                searchText,
                replaceText
            );

        cout << "\nReplacements made: "
             << count << endl;

        cout << "\nUpdated document:" << endl;
        cout << document;
    }
    else
    {
        cout << "Invalid choice." << endl;
    }

    return 0;
}