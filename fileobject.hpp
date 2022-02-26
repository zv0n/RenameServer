#ifndef FILEOBJECT_H
#define FILEOBJECT_H

#include <string>

enum FileType {
    TYPE_FILE,
    TYPE_DIRECTORY,
};

class FileObject {
public:
    FileObject( const std::string &name )
            : _name( name ) {}

    const std::string &getName() const {
        return _name;
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

    bool operator < (const FileObject &other) const {
        return _name < other.getName();
    }

    bool operator > (const FileObject &other) const {
        return _name > other.getName();
    }

private:
    std::string _name;
    int _depth = -1;
    FileType _type = TYPE_FILE;
};
#endif
