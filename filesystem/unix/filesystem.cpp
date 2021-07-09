#include "../filesystem.hpp"
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#ifdef __APPLE__
#include <sys/syslimits.h>
#endif
#include <stdexcept>
#include <iostream>

char FSLib::dir_divisor = '/';

FSLib::Directory::Directory( const string &path_ ) : dir_path( path_ ) {}

FSLib::Directory::Iterator::Iterator( const Directory &d_ )
        : d( opendir( d_.path() ) ) {
    if ( !exists( d_.path() ) || !isDirectory( d_.path() ) ) {
        throw std::runtime_error(
            std::string( "Directory " ) + d_.path() +
            " either doesn't exist or isn't a directory" );
    }

    current_entry = readdir( d );

    // skip "." and ".."
    if ( current_entry != nullptr &&
         ( !strcmp( current_entry->d_name, "." ) ||
           !strcmp( current_entry->d_name, ".." ) ) )
        ++( *this );
}

FSLib::Directory::Iterator::Iterator( const Directory &d_,
                                      const struct dirent *current_entry_ )
        : d( opendir( d_.path() ) ), current_entry( current_entry_ ) {}

FSLib::Directory::Iterator::~Iterator() {
    if ( d )
        closedir( d );
}

bool FSLib::exists( const string &path ) {
    struct stat path_stat;
    return stat( path.c_str(), &path_stat ) == 0;
}

string FSLib::canonical( const string &path ) {
    char_t *canonical_path = new char_t[PATH_MAX];
    auto failed = realpath( path.c_str(), canonical_path ) == nullptr;

    if ( failed ) {
        delete[] canonical_path;
        return string();
    }

    string canonical_string{ canonical_path };
    delete[] canonical_path;
    return canonical_string;
}

bool FSLib::isDirectory( const string &path ) {
    struct stat path_stat;

    if ( stat( path.c_str(), &path_stat ) != 0 )
        return false;

    return S_ISDIR( path_stat.st_mode );
}

bool FSLib::rename( const string &file_a, const string &file_b ) {
    // TODO log
    std::cout << file_a << " -> " << file_b << std::endl;
    return ::rename( file_a.c_str(), file_b.c_str() ) == 0;
}

// TODO do windows version
bool deleteRecursive(const string &dir) {
    for(const auto &file : FSLib::Directory(dir)) {
        auto path = dir + "/" + file;
        if(FSLib::isDirectory(path)) {
            if(!deleteRecursive(path)) {
                return false;
            }
        } else if(unlink(path.c_str()) != 0) {
            return false;
        }
    }
    return rmdir(dir.c_str()) == 0;
}

bool FSLib::deleteFile( const string &file ) {
    // TODO log
    auto canon = canonical( file );
    if(canon.empty()) {
        return false;
    }

    if(isDirectory(canon)) {
        return deleteRecursive(canon);
    }
    return unlink(canon.c_str()) == 0;
}

bool FSLib::createDirectoryFull( const string &path ) {
    uint64_t pos{};
    // go through all directories leading to the last one
    // and create them if they don't exist
    do {
        // get partial directory path
        pos = path.find_first_of( "/", pos );
        if ( pos > 0 ) {
            auto dirname = path.substr( 0, pos );
            // create it if it doesn't exist
            if ( !FSLib::exists( dirname ) ) {
                if ( mkdir( dirname.c_str(),
                            S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH ) != 0 )
                    return false;
            }
        }
        pos++;
    } while ( pos < path.length() && pos != 0 );
    return true;
}

FSLib::Directory::iterator FSLib::Directory::end() {
    return Iterator( *this, nullptr );
}

FSLib::Directory::const_iterator FSLib::Directory::end() const {
    return Iterator( *this, nullptr );
}

char_t const *FSLib::Directory::Iterator::operator*() const {
    return current_entry->d_name;
}

FSLib::Directory::Iterator &FSLib::Directory::Iterator::operator++() {
    if ( current_entry == nullptr )
        return *this;
    current_entry = readdir( d );
    // skip . and ..
    if ( current_entry != nullptr &&
         ( !strcmp( current_entry->d_name, "." ) ||
           !strcmp( current_entry->d_name, ".." ) ) )
        return operator++();
    return *this;
}

bool FSLib::Directory::Iterator::operator==( const Iterator &i_other ) const {
    return i_other.current_entry == current_entry;
}

string FSLib::getContainingDirectory( const string &path ) {
    auto pos = path.find_last_of('/');
    if(pos == string::npos) {
        return ".";
    }
    return path.substr(0, pos);
}

string FSLib::getFileName( const string &path ) {
    auto pos = path.find_last_of('/');
    if(pos == string::npos) {
        return path;
    }
    return path.substr(pos+1);
}

string FSLib::getFileExtension( const string &path ) {
    auto pos = path.find_last_of('.');
    if(pos == string::npos) {
        return "";
    }
    return path.substr(pos + 1);
}
