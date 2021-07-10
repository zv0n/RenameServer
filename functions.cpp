#include <algorithm>
#include <dlfcn.h>
#include <iostream>
#include "functions.hpp"
#include "filesystem/filesystem.hpp"

std::vector< RenameLibrary > getLibraries(const std::vector<std::pair<std::string,std::string>> &libraries) {
    // TODO get from config file
    std::vector< RenameLibrary > result{};
    for ( auto &library : libraries ) {
        void *libhndl = dlopen( library.first.c_str(), RTLD_NOW );
        if ( !libhndl ) {
            std::cerr << "Could not load library " << std::get<0>(library) << std::endl;
            closeLibraries( result );
            return {};
        }
        RenameLibrary rl;
        rl.libhndl = libhndl;
        rl.init = ( bool ( * )(const std::string &) )dlsym( libhndl, "init" );
        if ( !rl.init ) {
            result.push_back( rl );
            goto dlsymerror;
        }
        rl.getOptions = ( std::vector< RenameObject >( * )(
            const RenameObject & ) )dlsym( libhndl, "getOptions" );
        if ( !rl.getOptions ) {
            result.push_back( rl );
            goto dlsymerror;
        }
        rl.renamePath =
            ( bool ( * )( const std::string &, const RenameObject & ) )dlsym(
                libhndl, "renamePath" );
        if ( !rl.renamePath ) {
            result.push_back( rl );
            goto dlsymerror;
        }
        rl.getCustomKeys = ( std::vector< std::string >( * )() )dlsym(
            libhndl, "getCustomKeys" );
        if ( !rl.getCustomKeys ) {
            result.push_back( rl );
            goto dlsymerror;
        }
        rl.getName = ( const std::string(*)() ) dlsym( libhndl, "getName" );
        rl.config = library.second;
        result.push_back( rl );
    }
    return result;

dlsymerror:
    std::cerr << "dlsym: " << dlerror() << std::endl;
    closeLibraries( result );
    return {};
}

void closeLibraries( std::vector< RenameLibrary > &libraries ) {
    for ( auto &library : libraries ) {
        dlclose( library.libhndl );
    }
}

void addFilesRecursive(const std::string &prefix, std::vector< std::string > &results, const std::string &filename, const std::string &containing_directory, bool dir_only = false ) {
    auto path = containing_directory + FSLib::dir_divisor + filename;
    if(!dir_only || FSLib::isDirectory(path)) {
        results.push_back(prefix + filename);
    }
    if( FSLib::isDirectory(path) ) {
        auto newprefix = prefix + filename + FSLib::dir_divisor;
        for(const auto &entry : FSLib::Directory(path)) {
            addFilesRecursive(newprefix, results, entry, path, dir_only);
        }
    }
}

std::vector< std::string > getFilesInSource( const std::string &source_dir ) {
    std::vector< std::string > result;
    for(const auto &entry : FSLib::Directory(source_dir)) {
        addFilesRecursive("", result, entry, source_dir);
    }
    std::sort(result.begin(), result.end());
    return result;
}

std::vector< std::string > getTargetDirectories( const std::string &target_dir ) {
    std::vector< std::string > result;
    for(const auto &entry : FSLib::Directory(target_dir)) {
        addFilesRecursive("", result, entry, target_dir, true);
    }
    std::sort(result.begin(), result.end());
    return result;
}

std::string safeJson(std::string input) {
    for(size_t i = 0; i < input.size(); i++) {
        if(input[i] == '\\') {
            input.insert(i, "\\");
            i++;
        }
        if(input[i] == '"') {
            input.insert(i, "\\");
            i++;
        }
    }
    return input;
}
