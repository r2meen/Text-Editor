#ifndef REPLACE_H
#define REPLACE_H

#include <cstddef>
#include <string>

class Replace {
public:
    // Replaces the first occurrence. Returns true if successful.
    static bool replaceFirst(
        std::string& document,
        const std::string& oldText,
        const std::string& newText
    );

    // Replaces all occurrences and returns the replacement count.
    static std::size_t replaceAll(
        std::string& document,
        const std::string& oldText,
        const std::string& newText
    );
};

#endif