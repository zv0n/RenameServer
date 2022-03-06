#ifndef RENAME_LIBRARY_H
#define RENAME_LIBRARY_H

#include <vector>
#include "rename_object.hpp"

struct RenameLibrary {
    bool ( *init )( const std::string & );
    std::vector< RenameObject > ( *getOptions )( const RenameObject & );
    bool ( *renamePath )( const std::string &, const RenameObject & );
    std::vector< std::pair< std::string, std::string > > ( *getCustomKeys )();
    const std::string ( *getName )();
    const bool ( *canRenameMultipleFiles )();
    void *libhndl;
    std::string name;
    std::string config;
};

#endif
