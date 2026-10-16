#include "SearchReplace.h"

SearchReplace::SearchReplace()
{
    capacity = 5;
    size = 0;
    results = new SearchResult[capacity];
}

SearchReplace::~SearchReplace()
{
    delete[] results;
}

void SearchReplace::resizeArray()
{
    capacity = capacity * 2;

    SearchResult* newResults = new SearchResult[capacity];

    for (int i = 0; i < size; i++)
    {
        newResults[i] = results[i];
    }

    delete[] results;
    results = newResults;
}

void SearchReplace::clearResults()
{
    size = 0;
}

void SearchReplace::search(string document[], int totalLines, string word)
{
    clearResults();

    for (int i = 0; i < totalLines; i++)
    {
        int position = 0;

        while ((position = document[i].find(word, position)) != string::npos)
        {
            if (size == capacity)
            {
                resizeArray();
            }

            results[size].line = i;
            results[size].position = position;
            size++;

            position = position + word.length();
        }
    }
}

void SearchReplace::replaceOne(string document[], int totalLines, string word, string replacement, int occurrence)
{
    search(document, totalLines, word);

    if (occurrence < 0 || occurrence >= size)
    {
        return;
    }

    int line = results[occurrence].line;
    int position = results[occurrence].position;

    document[line].replace(position, word.length(), replacement);
}

void SearchReplace::replaceAll(string document[], int totalLines, string word, string replacement)
{
    for (int i = 0; i < totalLines; i++)
    {
        int position = 0;

        while ((position = document[i].find(word, position)) != string::npos)
        {
            document[i].replace(position, word.length(), replacement);

            position = position + replacement.length();
        }
    }
}

SearchResult* SearchReplace::getResults()
{
    return results;
}

int SearchReplace::getSize()
{
    return size;
}