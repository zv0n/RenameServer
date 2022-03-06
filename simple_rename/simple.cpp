#include "../filesystem/filesystem.hpp"
#include "../library.hpp"

#include <sstream>
#include <vector>
#include <iostream>

#ifdef _WIN32

#include <windows.h>

#include <codecvt>
#include <fcntl.h>
#include <io.h>
#include <locale>

constexpr const char_t *_tv_rename_dir_divider = L"\\";

#else

constexpr const char_t *_tv_rename_dir_divider = "/";

#endif

bool init( const string &config_path ) {
    return true;
}

std::vector< RenameObject > getOptions( const RenameObject & /*UNUSED*/ ) {
    return {};
}

bool renamePath( const string &path, const RenameObject &renamer ) {
    string new_name = "";

    if ( renamer.getCustomFields().find( "new_name" ) !=
         renamer.getCustomFields().end() ) {
        new_name = renamer.getCustomFields().at( "new_name" );
    } else {
        return false;
    }

    if ( new_name.find( '/' ) != string::npos ||
         new_name.find( '\\' ) != string::npos ) {
        return false;
    }
    return FSLib::rename(
        path, FSLib::canonical( FSLib::getContainingDirectory( path ) ) + "/" +
                  new_name );
}

std::vector< std::pair< string, string > > getCustomKeys() {
    return { { "new_name", STRING_TYPE } };
}

const string getName() {
    return "Simple Rename";
}

const bool canRenameMultipleFiles() {
    return false;
}
