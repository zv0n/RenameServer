#ifndef SIMPLE_RENAME_HPP
#define SIMPLE_RENAME_HPP

#include <vector>

#include "rename_object.hpp"

#ifdef _WIN32

using string = std::wstring;

#else

using string = std::string;

#endif

#define STRING_TYPE "string"
#define NUM_TYPE "number"
#define YEAR_TYPE "year"
#define DATE_TYPE "date"
#define BOOL_TYPE "bool"
#define MULTICHOICE_TYPE "multichoice"

extern "C" {
bool init(const string &configuration);
std::vector< RenameObject > getOptions( const RenameObject &search );
bool renamePath( const string &path, const RenameObject &renamer );
// TODO create object
std::vector< std::unordered_map<string, string> > getCustomKeys();
// option internal representation, option display value
std::vector< std::pair<string, string> > getCustomKeyOptions(const string &key);
const string getCustomKeyDefault(const string &key);
const string getName();
const bool canRenameMultipleFiles();
}

#endif // TV_RENAME_HPP
