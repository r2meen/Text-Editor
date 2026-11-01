#include "FileManager.h"

#include <fstream>
#include <vector>

FileManager::FileManager() : currentFile(), modified(false) {}

void FileManager::newDocument(Document& doc) {
    doc.clear();
    currentFile.clear();
    modified = false;
}

void FileManager::newDocument(Document& doc, const std::string& filename) {
    doc.clear();
    currentFile = filename;
    modified = false;
}

bool FileManager::openFile(const std::string& path, Document& doc) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        // Normalize Windows CRLF line endings.
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }

    if (file.bad()) {
        return false;
    }

    // Only replace the current document after the file has been read fully.
    doc.clear();
    for (const std::string& text : lines) {
        doc.appendLine(text);
    }

    currentFile = path;
    modified = false;
    return true;
}

bool FileManager::save(const Document& doc) {
    if (currentFile.empty()) {
        return false;
    }
    return saveAs(currentFile, doc);
}

bool FileManager::saveAs(const std::string& path, const Document& doc) {
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }

    for (int i = 0; i < doc.getLineCount(); ++i) {
        file << doc.getLine(i) << '\n';
    }
    file.flush();
    if (!file) {
        return false;
    }

    currentFile = path;
    modified = false;
    return true;
}

bool FileManager::hasFileName() const {
    return !currentFile.empty();
}

std::string FileManager::getFileName() const {
    return currentFile;
}

bool FileManager::isModified() const {
    return modified;
}

void FileManager::markModified() {
    modified = true;
}
