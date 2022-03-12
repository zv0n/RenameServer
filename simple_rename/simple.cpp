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
    string new_name = renamer.getPresentedName();
    if(new_name.empty()) {
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

std::vector< std::unordered_map< string, string > > getCustomKeys() {
    return {};
}

std::vector< std::pair< string, string > > getCustomKeyOptions(const string &key) {
    return {};
}

const string getCustomKeyDefault(const string &key) {
    return "";
}

const string choiceDisplay() {
    return "\%name";
}

const string getName() {
    return "Simple Rename";
}

const bool canRenameMultipleFiles() {
    return false;
}

const bool shouldPerformSearch() {
    return false;
}
