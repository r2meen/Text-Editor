#ifndef SEARCH_REPLACE_H
#define SEARCH_REPLACE_H

#include <string>
using namespace std;

struct SearchResult
{
    int line;
    int position;
};

class SearchReplace
{
private:
    SearchResult* results;
    int size;
    int capacity;

    void resizeArray();

public:
    SearchReplace();
    ~SearchReplace();

    void clearResults();
    void search(string document[], int totalLines, string word);
    void replaceOne(string document[], int totalLines, string word, string replacement, int occurrence);
    void replaceAll(string document[], int totalLines, string word, string replacement);

    SearchResult* getResults();
    int getSize();
};

#endif