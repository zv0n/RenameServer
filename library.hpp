#ifndef SIMPLE_RENAME_HPP
#define SIMPLE_RENAME_HPP

#include <vector>

#include "rename_object.hpp"

#ifdef _WIN32

using string = std::wstring;

#else

using string = std::string;

#endif

extern "C" {
bool init(const string &configuration);
std::vector< RenameObject > getOptions( const RenameObject &search );
bool renamePath( const string &path, const RenameObject &renamer );
std::vector< string > getCustomKeys();
const string getName();
const bool canRenameMultipleFiles();
}

#endif // TV_RENAME_HPP
