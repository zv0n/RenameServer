#ifndef FILEOBJECT_H
#define FILEOBJECT_H

#include <string>

enum FileType {
    TYPE_FILE,
    TYPE_DIRECTORY,
};

class FileObject {
public:
    const std::string &getPath() const {
        return _path;
    }

    const int &getDepth() const {
        return _depth;
    }

    const FileType &getFileType() const {
        return _type;
    }

    void setFileType(FileType file_type) {
        _type = file_type;
    }

    void setDepth(int depth) {
        _depth = depth;
    }

    void setPath(const std::string &path) {
        _path = path;
    }

    bool operator < (const FileObject &other) const {
        return _path < other.getPath();
    }

    bool operator > (const FileObject &other) const {
        return _path > other.getPath();
    }

private:
    std::string _path = "";
    int _depth = -1;
    FileType _type = TYPE_FILE;
};
#endif
