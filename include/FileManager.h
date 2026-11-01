#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>
#include "Document.h"

// Handles file I/O and tracks the document's filename and unsaved state.
class FileManager {
private:
    std::string currentFile;
    bool modified;

public:
    FileManager();

    // Start a new, unnamed document.
    void newDocument(Document& doc);

    // Start a new document associated with a filename (written on Save).
    void newDocument(Document& doc, const std::string& filename);

    // Load a file. On failure, the existing document is left unchanged.
    bool openFile(const std::string& path, Document& doc);

    // Save to the current filename, or save under a new filename.
    bool save(const Document& doc);
    bool saveAs(const std::string& path, const Document& doc);

    bool hasFileName() const;
    std::string getFileName() const;
    bool isModified() const;
    void markModified();
};

#endif
