#ifndef TEXT_UTILS_H
#define TEXT_UTILS_H

#include <cctype>
#include <cstddef>
#include <string>

// How a search / replace pattern should be matched.
struct MatchOptions {
    bool wholeWord = false;      // false = any sequence of characters
    bool caseSensitive = false;
};

// Shared by Search.cpp and Replace.cpp (previously copy-pasted in both).
namespace TextUtils {

inline bool isWordCharacter(char ch) {
    return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_';
}

inline bool charsEqual(char a, char b, bool caseSensitive) {
    if (caseSensitive)
        return a == b;
    return std::tolower(static_cast<unsigned char>(a)) ==
           std::tolower(static_cast<unsigned char>(b));
}

// Does `pattern` appear in `text` starting exactly at `position`?
// Worst case O(m) where m = pattern length.
inline bool matchesAt(const std::string& text, std::size_t position,
                      const std::string& pattern, bool caseSensitive) {
    if (position + pattern.length() > text.length())
        return false;

    for (std::size_t j = 0; j < pattern.length(); j++) {
        if (!charsEqual(text[position + j], pattern[j], caseSensitive))
            return false;
    }
    return true;
}

// Is the match at `position` surrounded by non-word characters?
inline bool isWholeWord(const std::string& text, std::size_t position,
                        std::size_t length) {
    bool left = position == 0 || !isWordCharacter(text[position - 1]);

    std::size_t end = position + length;
    bool right = end >= text.length() || !isWordCharacter(text[end]);

    return left && right;
}

inline bool isMatch(const std::string& text, std::size_t position,
                    const std::string& pattern, const MatchOptions& opts) {
    if (!matchesAt(text, position, pattern, opts.caseSensitive))
        return false;

    return !opts.wholeWord || isWholeWord(text, position, pattern.length());
}

} // namespace TextUtils

#endif
