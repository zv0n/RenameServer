#ifndef RENAME_LIBRARY_H
#define RENAME_LIBRARY_H

#include <vector>
#include "rename_object.hpp"

struct RenameLibrary {
    bool ( *init )( const std::string & );
    std::vector< RenameObject > ( *getOptions )( const RenameObject & );
    bool ( *renamePath )( const std::string &, const RenameObject & );
    std::vector< std::string > ( *getCustomKeys )();
    void *libhndl;
    std::string name;
    std::string config;
};

#endif
