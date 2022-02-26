#ifndef RENAME_FUNCTIONS_H
#define RENAME_FUNCTIONS_H

#include <vector>
#include "rename_library.hpp"
#include "fileobject.hpp"

std::vector< RenameLibrary > getLibraries(const std::vector<std::pair<std::string,std::string>> &libraries);
void closeLibraries( std::vector< RenameLibrary > &libraries );
std::vector< FileObject > getFilesInSource( const std::string &source_dir );
std::vector< FileObject > getTargetDirectories( const std::string &target_dir );
std::string safeJson(std::string input);

#endif
