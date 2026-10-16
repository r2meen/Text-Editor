#include <iostream>
#include "../include/SearchReplace.h"

int main()
{
    SearchReplace sr;

    string document[3] = {
        "hello world",
        "hello everyone",
        "world of hello"
    };

    sr.search(document, 3, "hello");

    cout << "Occurrences found: " << sr.getSize() << endl;

    sr.replaceOne(document, 3, "hello", "hi", 0);

    cout << "\nAfter replacing one occurrence:\n";

    for (int i = 0; i < 3; i++)
    {
        cout << document[i] << endl;
    }

    sr.replaceAll(document, 3, "world", "universe");

    cout << "\nAfter replacing all occurrences of world:\n";

    for (int i = 0; i < 3; i++)
    {
        cout << document[i] << endl;
    }

    return 0;
}